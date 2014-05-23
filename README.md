Boodew
======

Public domain lisp-like cube-script-inspired, dynamically scoped, functional and very
slow language based on string evaluation and string substitution.

This is a variant of the scripting language used in sauerbraten
(http://sauerbraten.org) and tesseract (http://tesseract.gg)

Principle
---------
The language is made out of 3 fundamental operators:

* `()` executes what is enclosed
* `[]` is a nested string which is not evaluated except by builtins that execute
  it
* `@` is very similar to Lisp unquote operator but here applied to string
  instead of lists. When followed by `(`, it executes what contained in `(...)`
  and substitutes `@(...)` by the result of the execution. Syntax
  however differs from cube script. In cube script the number of `@` indicates
  the level of nesting where to execute while in boodew, `@` behaves as an
  escape character i.e. one `@` will execute right away if followed by `(`,
  while two `@`s will be evaluated into one `@` first. See the tests for more
  details

With these three operators, we then specify builtins that implements classical
functionalities, like loops, conditionals.

Boodew is dynamically scoped. This is nasty but easy to implement. Function
arguments is straightfoward. At each function call, we create variables called
`0`, `1`, .... `$` operator is able to get back with `($ bla)` syntax.

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

How to build
------------
The code was tested on linux with gcc 4.8.2 and clang 3.4. It was also tested
sometime ago with vs2013. Build may break though since I did not test recently

Examples
--------

### Imperative example
Let's look at a very simple imperative example:

`loop i 16 [? (< ($ i) 8) [echo ($ i)] [continue]]`

Here we simply write a loop that iterates 16 times and print something for the
first 8 iterations. This line is one expression. The first argument always
defines the function or the builtin to call here `loop`.
The next argument is the loop variable and the next one is the maximum loop
counter. This loop goes from 0 to 15. The last element enclosed in brackets in
the expression to execute. Brackets mean that the expression evaluation is not
done when parsing the loop expression but when loop builtin will decide to
execute it. Let's break it into pieces:

* `?` defines a condition statement made
of the condition which is executed before `?` is run, the `then` expression and
an optional `else` expression `(< ($ i) 8)`

* `<` is comparison. Here we use `$` that looks up a variable and replaces it by
its value

* The two last statements are the `then` expression and the `else` one. Note
that loops support classical break and continue

### Function definition and function call

Consider this example:

`var fn [echo ($ in_upper_scope)]; (var in_upper_scope bim); ($ fn)`

As you see, you define a function and use it. The function by itself is no more
than a string i.e. a `[...]` expression. Calling it consists in looking up the
function variable i.e. its string and to evaluate it. This example also shows a
simple usage of dynamic scoping where the function can access the local
variables of its caller(s).

Function arguments are easy since they are just regular variables created on the
fly by the interpreter. Consider this example:

`var fn [echo ($ 0)]; ($ fn) boum`

We define a function called `fn` that implicitly has one argument. This argument
can fetched by its name `0` using the regular `$` builtin. Extra arguments will
be called `1`, `2`...
This example will therefore print "boum"

### String substitution
Look at the other example:

`var i 4; echo [hop@(int ($ i))]`

Here we use operator `@`. This operator forces the evaluation inside a nested
`[]` expression. Therefore `int ($ i)` will be evaluated just before echo is
executed. Since i equals 4, echo will output "hop4"

Note that `@` also works as an escape character and can therefore be used to
delay the string expansion. Consider this example:

`var i 4; echo [hop@@($ i)]`

It will print "[hop@($ i)]" since the first `@` will prevent the evaluation of
the second `@`. After the expansion, the `@` expression can then be evaluated as
above.

### Higher order programming
Function declaration and string substitution are enough to define a real
functional language.

For example, consider this small code:

`var bind [^ [[@@($ 0) @@($ 1) ($ 0)]]]`

`var plus (($ bind) + 2)`

`echo (($ plus) 3)`

It defines a function currying function i.e. a function that partially applies
arguments to another function, similar to what ML languages do or what you can
do with std::bind in c++.  Let's examine it:

* First, we define our higher-order bind function. As the rest in boodew, this
  is just a string. The basic idea of it is given a function (its first argument)
  and the argument to apply (its second argument), we build a new string that
  will define a function where the first argument is applied. To do so, we use
  operator `@`. Note that we use the "double" version of it here, since we need
  to delay its evaluation once.

* Secondly, we use operator `^`  which is the identity function that simply
  returns its argument. That makes sense since the argument is a string i.e. this
  is the function we want to create.  Both `^` and `@` can define a function of
  functions since `^` will return a string modified by the later application of
  `@` We then apply this operator on builtin `+` applying the argument `2` and we
  create a new function (i.e. still a string) out of it

* We finally call this function. Nothing special here. This is a regular
  function Note that we could be even more brutal with this concept by making
  partial evaluation lazy. Using more operator `@`, we can make the application
  both partial and lazy by letting the final evaluation being done at call time.
  Anything is possible since we just assemble strings.

What next?
----------
New builtins can be easily added to process arrays. One interesting addition
would also to add a proper user-data value (on top of string, double and bool)
to have really interesting patterns.
