/*-------------------------------------------------------------------------
 - boodew - a very simple and small (and slow) language based on strings
 -------------------------------------------------------------------------*/
#include <functional>
#include <string>
#include <vector>

#define BDW_JOIN(x, y) _BDW_JOIN(x, y)
#define _BDW_JOIN(x, y) _BDW_JOIN2(x, y)
#define _BDW_JOIN2(x, y) x##y

namespace boodew {

// functions and builtins can have a variable number of arguments
typedef const std::vector<std::string> &args;
typedef std::function<std::string(args)> builtin_type;
typedef std::function<std::string()> cvar_type;

// format string helper
std::string format(const char *fmt, ...);

// extract the argument with extra checks
const std::string &get(args arg, size_t idx);

// append a new builtin in the (global and shared) boodew context
bool new_builtin(const std::string&, const builtin_type&);

// helper macros to do in c++ pre-main (oh yeah)
#define CMDL(N, FN) static auto BDW_JOIN(builtin,__COUNTER__) = new_builtin(N,FN);
#define CMDN(N, FN) static auto BDW_JOIN(builtin,FN) = new_builtin(N,FN);
#define CMD(N) CMDN(N,#N)

// append a global variable
bool new_cvar(const std::string&, const cvar_type&, const builtin_type&);

// helper macros to do at pre-main (integer value here)
#define IVAR(N,MIN,CURR,MAX) int N = CURR;\
  static auto BDW_JOIN(cvar,__COUNTER__) =\
    boodew::new_cvar(#N,[](){return std::to_string(N);}, [](boodew::args arg) {\
      const auto x = std::stoi(boodew::get(arg,1));\
      if (x>=MIN && x<=MAX) N=x;\
      else std::cerr<<boodew::format("range for %s is (%d,%d)",#N,MIN,MAX)<<std::endl;\
      return boodew::get(arg,1);\
    });
std::pair<std::string,bool> exec(const std::string&);
} // namespace boodew

