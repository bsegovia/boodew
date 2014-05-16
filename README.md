Boodew
======

Public domain lisp/cube-script inspired, dynamically scoped, functional and very
slow language based on string evaluation and string substitution.

This is a variant of the scripting language used in sauerbraten
(http://sauerbraten.org) and tesseract (http://tesseract.gg)

Principle
---------
The language is made out of 3 fundamental operators:

* '()' executes what is enclosed
* '[]' is a nested string which is not evaluated except by builtins that execute
  it
* '@' is very similar to Lisp unquote operator but here applied to string
  instead of lists. When followed by '(', it executes what contained in '(...)'
  and substitutes '@(...)' by the result of the execution. Syntax
  however differs from cube script. In cube script the number of '@' indicates
  the level of nesting where to execute while in boodew, '@' behaves as an
  escape character i.e. one '@' will execute right away if followed by '(',
  while two '@'s will be evaluated into one '@' first. See the tests for more
  details

With these three operators, we then specify builtins that implements classical
functionalities, like loops, conditionals.

Boodew is dynamically scoped. This is nasty but easy to implement. Function
arguments is straightfoward. At each function call, we create variables called
'0', '1', .... '$' operator is able to get back with '($ bla)' syntax.

Interestingly, expressions execute until fixed point i.e. when a literal is
found because a literal is invariant when evaluated

Implementation
--------------
The implementation aims at being as small as possible using c++ as much as we
can: exceptions are abused to get proper control flow, we extensively use std
and so on...

Usage
-----
The code has been quickly tested with simple unit tests that you may find in
'tests.cpp'. You will find that some constructs like function currying can
easily done with the language.

What next?
----------
New builtins can be easily added to process arrays. One interesting would also
to add a proper user-data value (on top of string, double and bool) to have
really interesting patterns.

