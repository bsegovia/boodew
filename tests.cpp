#include <iostream>
#include "boodew.hpp"
using namespace std;

static void run(const char *str, const char *expected = NULL, bool fails=false) {
  cout << "running '" << str << "'";
  const auto r = script::exec(str);
  if (!r.second != fails)
    cout << " failed with " << r.first << endl;
  else
    cout << " succeeded" << endl;
}

int main(int argc, const char *argv[]) {
  run("var i; echo [hop@($ i)]");
  run("echo ($ fov)");
  run("var f [echo (* ($ 0)($ 0))]; ($ f) 3");
  run("var x 1; var y 2; echo (+ ($ x))",NULL,true);
  run("var i 4; echo [hop@($ i)]");
  run("var i 4; echo [hop@@($ i)]");
  run("loop i 16 [? (< ($ i) 8) [echo ($ i)] [break]]");
  run("loop i 16 [? (< ($ i) 8) [echo ($ i)] [continue]]");
  run("echo deded; echo er");
  run("echo (+ 1 2)");
  run("echo (.. (.. a bb) (+ 1 2))");
  run("echo (.. (.. a bb) cc)");
  run("var fn [echo ($ 0)]; ($ fn) boum\n($ fn) boum2");
  run("# [this is a comment]; var fn [echo ($ 0)]; ($ fn) boum\n($ fn) boum2");
  run("var bind [do [return [[@@($ 0) @@($ 1) ($ 0)]]]]; echo ((($ bind) + 2) 3)");
  run("var bind [do [return [[@@($ 0) @@($ 1) ($ 0)]]]];"
      "var plus (($ bind) + 2);"
      "echo (do [return (($ plus) 3)])");
  return 0;
}

