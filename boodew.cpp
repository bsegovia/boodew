#include <cstdarg>
#include <iostream>
#include <unordered_map>
#include <iterator>
#include <sstream>
#include <algorithm>
#include "boodew.hpp"

namespace script {
using namespace std;
static bool stob(const string &s) { return 0!=stoi(s); }

#define range(v,m,M) for (int v = int(m); v < int(M); ++v)
#define rangei(m,M) range(i,m,M)
#define rangej(m,M) range(j,m,M)
#define loop(v,m) range(v,0,m)
#define loopi(m) loop(i,m)
#define loopj(m) loop(j,m)
#define JOIN(x, y) _DO_JOIN(x, y)
#define _DO_JOIN(x, y) _DO_JOIN2(x, y)
#define _DO_JOIN2(x, y) x##y

#define EX(NAME,TYPE,FIELD,WHAT)\
struct NAME : exception {\
  NAME(TYPE FIELD) : FIELD(FIELD) {}\
  ~NAME() throw() {}\
  const char *what() const throw() {return WHAT;}\
  TYPE FIELD;\
};
EX(return_branch, string, str, "")
EX(loop_branch, bool, is_break, "")
EX(script_exception, string, str, str.c_str())
#undef EX

static string format(const char *fmt, ...) {
  int size = 256;
  string str;
  va_list ap;
  for (;;) {
    str.resize(size);
    va_start(ap, fmt);
    const auto n = vsnprintf((char*)str.c_str(), size, fmt, ap);
    if (n < 0) throw script_exception("format string issue");
    va_end(ap);
    if (n < size) {
      str.resize(n);
      return str;
    }
    size = n+1;
  }
  return str;
}

template <typename T> T *insta() { static T internal; return &internal; }
const string &get(args arg, size_t idx) {
  if (arg.size() <= idx) throw script_exception("argument is missing");
  return arg[idx];
}
typedef unordered_map<string, function<string(args)>> builtinmap;
bool new_builtin(const string &n, const function<string(args)> &fn) {
  return insta<builtinmap>()->insert(make_pair(n,fn)), true;
}

// we wrap console global variables with builtins
typedef unordered_map<string, function<string()>> cvar_map;
bool new_cvar(const string &n, const function<string()> &f0, const function<string(args)> &f1) {
  return insta<cvar_map>()->insert(make_pair(n,f0)), new_builtin(n,f1);
}

// (scoped) local variables
typedef vector<unordered_map<string,string>> stack;
static string new_local(const string &name, const string &value) {
  const auto s = insta<stack>();
  if (s->size() == 0) s->push_back(unordered_map<string,string>());
  return s->back()[name] = value;
}
struct scope {
  scope() {insta<stack>()->push_back(unordered_map<string,string>());}
  ~scope() {insta<stack>()->pop_back();}
};

static string getvar(args arg) {
  for (auto it = insta<stack>()->rbegin(); it != insta<stack>()->rend(); ++it) {
    auto local = it->find(get(arg,1));
    if (local != it->end()) return local->second;
  }
  const auto it = insta<cvar_map>()->find(get(arg,1));
  if (it != insta<cvar_map>()->end()) return it->second();
  throw script_exception(format("unknown identifier %s", get(arg,1).c_str()));
}

static string ex(const string &s, size_t curr=0);
static pair<string,size_t> expr(const string &s, char c, size_t curr) {
  const char *match = c=='['?"[]@":"()";
  stringstream ss;
  size_t opened = 1, next = curr;
  while (opened) {
    if ((next = s.find_first_of(match,next+1)) == string::npos)
      throw script_exception(format("missing %c", c=='['?']':')'));
    if (c == '[' && s[next] == '@') {
      ss << s.substr(curr+1, next-curr-1);
      if (s[next+1] == '(') {
        const auto v = expr(s, '(', next+1);
        ss << v.first;
        if (s[v.second]!=']') ss << s[v.second];
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
    if (tok.size() == 0) throw script_exception("missing identifer");
    auto const it = insta<builtinmap>()->find(tok[0]);

    // try a function call
    if (it==insta<builtinmap>()->end()) {
      if (insta<stack>()->size() > 1024)
        throw script_exception(format("stack overflow with %s",tok[0].c_str()));
      scope frame;
      rangei(1,tok.size()) new_local(to_string(i-1),tok[i]);
      ret = ex(tok[0]);
    } else
      ret = it->second(tok);
  }
  return ret;
}
static string while_builtin(args arg) {
  string last;
  while (stob(ex(get(arg,1)))) try { return last=ex(get(arg,2)); }
    catch (loop_branch e) { if (e.is_break) break; else continue; }
  return last;
}
static string loop_builtin(args arg) {
  scope frame;
  string last;
  auto const n = int(stod(get(arg,2)));
  loopi(n) {
    try { new_local(get(arg,1), to_string(i)); last = ex(get(arg,3)); }
    catch (loop_branch e) { if (e.is_break) break; else continue; }
  }
  return last;
}
#define O(S)CMDL(#S,[](args arg){return to_string(stod(get(arg,1)) S stod(get(arg,2)));})
O(+) O(-) O(/) O(*) O(==) O(!=) O(<) O(>) O(<=) O(>=)
#undef O
CMDL("var",[](args arg){return new_local(get(arg,1),arg.size()<3?"0":get(arg,2));})
CMDL("#", [](args){return "";})
CMDL("..", [](args arg){return get(arg,1)+get(arg,2);})
CMDL("echo", [](args arg){cout<<get(arg,1);return get(arg,1);})
CMDL("?", [](args arg){return stob(get(arg,1)) ? ex(get(arg,2)): ex(get(arg,3));})
CMDL("return", [](args arg)->string {throw return_branch(get(arg,1));})
CMDL("do", [](args arg){try {return ex(get(arg,1));} catch (return_branch e) {return e.str;}})
CMDL("break", [](args arg)->string {throw loop_branch(true);})
CMDL("continue", [](args arg)->string {throw loop_branch(false);})
CMDN("while", while_builtin)
CMDN("loop", loop_builtin)
CMDN("$", getvar)

pair<string,bool> exec(const string &s) {
  try { ex(s,0); return make_pair("",true); }
  catch (script_exception e) { return make_pair(string(e.what()),false); }
}
} /* namespace script */

