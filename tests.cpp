/*-------------------------------------------------------------------------
 - run a bunch of very simple tests for basic sanity check
 -------------------------------------------------------------------------*/
#include <iostream>
#include "boodew.hpp"
using namespace std;

static void run(const char *str, bool fails=false) {
  const auto r = boodew::exec(str);
  if (!r.second != fails)
    cout << "FAILED with " << r.first;
  if (!fails)
    cout << endl;
  else
    cout << r.first << endl;
}

// create a global variable
IVAR(some_global, 10, 30, 50);

int main(int argc, const char *argv[]) {
  // global variable
  run("echo ($ some_global)");
  run("var f [echo (* ($ 0)($ 0))]; ($ f) 3");
  run("var x 1; var y 2; echo (+ ($ x))",true);

  // string substitution
  run("var i; echo [hop@($ i)]");
  run("var i 4; echo [hop@(int ($ i))]");
  run("var str [[bla @(.. [echo] [ 4])]]; echo ($ str)");

  // delayed string substitution
  run("var i 4; echo [hop@[($ i)]");
  run("var i 4; echo [[hop@($ i)]]");

  // loop + control flow
  run("loop i 16 [? (< ($ i) 8) [echo ($ i)] [break]]");
  run("loop i 16 [? (< ($ i) 8) [echo ($ i)] [continue]]");
  run("echo (do [loop i 16 [? (!= ($ i) 8) [echo ($ i)] [return (int (- ($ i) 1))]]])");

  // dynamic scoping
  run("var fn [echo ($ in_upper_scope)]; (var in_upper_scope 3); (echo ($ in_upper_scope))");

  // simple echo
  run("echo deded; echo er");

  // simple operator
  run("echo (+ 1 2)");

  // string concat
  run("echo (.. (.. a bb) (+ 1 2))");
  run("echo (.. (.. a bb) cc)");

  // storing a function in a variable
  run("var fn [echo ($ 0)]; ($ fn) boum\n($ fn) boum2");
  run("# [this is a comment]; var fn [echo ($ 0)]; ($ fn) boum\n($ fn) boum2");

  // function arguments currying
  run("var bind [do [return [[@@($ 0) @@($ 1) ($ 0)]]]]; echo ((($ bind) + 2) 3)");
  run("var bind [do [return [[@@($ 0) @@($ 1) ($ 0)]]]];"
      "var plus (($ bind) + 2);"
      "echo (int (do [return (($ plus) 3)]))");

  return 0;
}

