/*-------------------------------------------------------------------------
 - boodew - a very simple and small (and slow) language based on strings
 -------------------------------------------------------------------------*/
#include <cstdarg>
#include <iostream>
#include <unordered_map>
#include <iterator>
#include <sstream>
#include <algorithm>
#include "boodew.hpp"

namespace boodew {
using namespace std;
static bool stob(const string &s) { return 0!=stoi(s); }

struct boodew_exception : exception {
  boodew_exception(string str) : str(str) {}
  ~boodew_exception() throw() {}
  const char *what() const throw() {return str.c_str();}
  string str;
};

string format(const char *fmt, ...) {
  int size = 256;
  string str;
  va_list ap;
  for (;;) {
    str.resize(size);
    va_start(ap, fmt);
    const auto n = vsnprintf((char*)str.c_str(), size, fmt, ap);
    if (n < 0) throw boodew_exception("format string issue");
    va_end(ap);
    if (n < size) {
      str.resize(n);
      return str;
    }
    size = n+1;
  }
  return str;
}

template <typename T> T *unique() { static T internal; return &internal; }
const string &get(args arg, size_t idx) {
  if (arg.size() <= idx) throw boodew_exception("argument is missing");
  return arg[idx];
}
typedef unordered_map<string, function<string(args)>> builtinmap;
bool new_builtin(const string &n, const builtin_type &fn) {
  return unique<builtinmap>()->insert(make_pair(n,fn)), true;
}

// we wrap console global variables with builtins
typedef unordered_map<string, function<string()>> cvar_map;
bool new_cvar(const string &n, const cvar_type &f0, const builtin_type &f1) {
  return unique<cvar_map>()->insert(make_pair(n,f0)), new_builtin(n,f1);
}

// (scoped) local variables
typedef vector<unordered_map<string,string>> stack;
static string new_local(const string &name, const string &value) {
  const auto s = unique<stack>();
  if (s->size() == 0) s->push_back(unordered_map<string,string>());
  return s->back()[name] = value;
}
struct scope {
  scope() {unique<stack>()->push_back(unordered_map<string,string>());}
  ~scope() {unique<stack>()->pop_back();}
};

static string getvar(args arg) {
  for (auto it = unique<stack>()->rbegin(); it != unique<stack>()->rend(); ++it) {
    auto local = it->find(get(arg,1));
    if (local != it->end()) return local->second;
  }
  const auto it = unique<cvar_map>()->find(get(arg,1));
  if (it != unique<cvar_map>()->end()) return it->second();
  throw boodew_exception(format("unknown identifier %s", get(arg,1).c_str()));
}

static string ex(const string &s, size_t curr=0);
static pair<string,size_t> expr(const string &s, char c, size_t curr) {
  const char *match = c=='['?"[]@":"()";
  stringstream ss;
  size_t opened = 1, next = curr;
  while (opened) {
    if ((next = s.find_first_of(match,next+1)) == string::npos)
      throw boodew_exception(format("missing %c", c=='['?']':')'));
    if (c == '[' && s[next] == '@') {
      ss << s.substr(curr+1, next-curr-1);
      if (s[next+1] == '(') {
        const auto v = expr(s, '(', next+1);
        ss << v.first;
        if (s[v.second]!=']'||opened!=1) ss << s[v.second];
        curr = v.second;
      } else {
        curr = next+1;
        while (s[++next]=='@') ss << '@';
      }
    } else
      opened += s[next] == c ? +1 : -1;
  }
  ss << s.substr(curr+1, next-curr-1);
  return make_pair((c=='[' ? ss.str() : ex(ss.str())), next+1);
}
static string ex(const string &s, size_t curr) {
  string ret, id;
  bool running = true;
  while (running) {
    vector<string> tok;
    for (;;) {
      const auto next = s.find_first_of("; \r\t\n[(", curr);
      const auto last = next == string::npos ? s.size() : next;
      const auto len = last-curr;
      const auto c = s[last];
      if (len!=0) tok.push_back(s.substr(curr,len));
      if (c == '(' || c == '[') {
        const auto v = expr(s, c, curr);
        tok.push_back(v.first);
        curr = v.second;
      } else if (c == ';' || c == '\n') {curr=last+1; break;}
      else if (c == '\0') {curr=last; running=false; break;}
      else curr=last+1;
    }

    // try to call a builtin
    if (tok.size() == 0) return string();
    auto const it = unique<builtinmap>()->find(tok[0]);

    // try a builtin call
    if (it!=unique<builtinmap>()->end())
      ret = it->second(tok);
    // we use fixed point to find literals on the fly!
    else if (tok.size() == 1 && tok[0] == s) return s;
    // this has to be a function call
    else {
      scope frame;
      for (size_t i = 1; i < tok.size(); ++i) new_local(to_string(i-1),tok[i]);
      ret = ex(tok[0]);
    }
  }
  return ret;
}
static string while_builtin(args arg) {
  string last;
  while (stob(ex(get(arg,1)))) try { return last=ex(get(arg,2)); }
    catch (bool b) { if (b) break; else continue; }
  return last;
}
static string loop_builtin(args arg) {
  scope frame;
  string last;
  auto const n = int(stod(get(arg,2)));
  for (int i = 0; i < n; ++i) {
    try { new_local(get(arg,1), to_string(i)); last = ex(get(arg,3)); }
    catch (bool b) { if (b) break; else continue; }
  }
  return last;
}
#define O(S)CMDL(#S,[](args arg){return to_string(stod(get(arg,1)) S stod(get(arg,2)));})
O(+) O(-) O(/) O(*) O(==) O(!=) O(<) O(>) O(<=) O(>=)
#undef O
CMDL("int",[](args arg){return to_string(stoi(arg[1]));})
CMDL("var",[](args arg){return new_local(get(arg,1),arg.size()<3?"0":get(arg,2));})
CMDL("#", [](args){return "";})
CMDL("..", [](args arg){return get(arg,1)+get(arg,2);})
CMDL("echo", [](args arg){cout<<get(arg,1);return get(arg,1);})
CMDL("?", [](args arg){return stob(get(arg,1)) ? ex(get(arg,2)): ex(get(arg,3));})
CMDL("return", [](args arg)->string {throw get(arg,1);})
CMDL("do", [](args arg){try {return ex(get(arg,1));} catch (string s) {return s;}})
CMDL("break", [](args arg)->string {throw true;})
CMDL("continue", [](args arg)->string {throw false;})
CMDN("while", while_builtin)
CMDN("loop", loop_builtin)
CMDN("$", getvar)

pair<string,bool> exec(const string &s) {
  try { ex(s,0); return make_pair("",true); }
  catch (const boodew_exception &e) { return make_pair(string(e.what()),false); }
}
} // namespace boodew

