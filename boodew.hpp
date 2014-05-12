#include <functional>
#include <string>
#include <vector>

namespace script {
typedef const std::vector<std::string> &args;
bool new_builtin(const std::string &n, const std::function<std::string(args)> &fn);
bool new_cvar(const std::string &n, const std::function<std::string()> &f0, const std::function<std::string(args)> &f1);
#define CMDL(N, FN) static auto JOIN(builtin,__COUNTER__) = new_builtin(N,FN);
#define CMDN(N, FN) static auto JOIN(builtin,FN) = new_builtin(N,FN);
#define CMD(N) CMDN(N,#N)

#define IVAR(N,MIN,CURR,MAX) int N = CURR;\
  static auto JOIN(cvar,__COUNTER__) = new_cvar(#N,[](){return to_string(N);},\
    [](args arg) {\
      const auto x = stoi(get(arg,1));\
      if (x>=MIN && x<=MAX) N=x;\
      else cerr << format("range for %s is (%d,%d)",#N,MIN,MAX) << endl;\
      return get(arg,1);\
    });
std::pair<std::string,bool> exec(const std::string&);
} /* namespace script */

