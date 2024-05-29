# Ship the programming language, once again!

#### This is probably my last implementation of ship. but now in C.

This project served as a learning project to learn the C language.\
I might have made some mistakes here and there, and I will continue to improve on this language.\
I will expand the language features here, once I finish most.\

The language is 100% custom-made, with its own VM and bytecode version.


## Features
Most of ship's statements are terminated using the `;` letter.

### Values
Ship has 4 types of values.
number, boolean, nil and object.\
strings, functions and classes are considered objects.


### Variables
Variables are essentially the same as in javascript. they are dynamic mutable.\
variables are created using the `var` keyword following the identifier name. 
```javascript
var my_name = "Ido";
var hobby = "coding";

hobby = 4;
my_name = nil;
```

### Loops
While loops are useful and are available in ship.
```javascript
while x > 3 {
    ...
}
```
Unfortunately, for loops are not supported in ship yet. but this is a planned feature.

### Conditionals
Ship uses a go like syntax, in order to create if statements.
```javascript
var a = 15;
if a >= 3 {
    ...
}
```

### Functions
Functions are pretty much the same as any other language.\
they are created using the `fn` keyword.
```rust
fn my_print(str) {
    print(str);
}
```
which then can be called using
```
my_print("Hello From Ship!");
```

### Errors
This is a feature that I worked hard to implement.\
ship has a great error information. and will try to guide you.\
\
for example:
```rust
fn get_max(a, b) {
    if a >= b {
        return a;
    }
    return b;
}

print(get_max100, 34));
```
Can you spot the error in the following ship code?\
Of course! the user has missed an opening `(`.\
Don't worry, ship got you covered.
```
error: Missing parentheses in call
    [main.ship:8:16]
    |
  8 | print(get_max100, 34));
    |             ^^^^
```
What a blast!

### Roadmap
- While loops (Done)
- Global and local variables (Done)
- Math expressions (Done)
- Functions (Done)
- Function arguments (Done)
- Garbage collector



## More Information

### Performance

Consider the following python code:
```python
def a():
    x = 100_000_000 # 1e08
    while x != 0:
        x = x - 1
a()
```

And it's .ship version:
```
fn a() {
    var x = 100000000;
    while x != 0 {
        x = x - 1;
    }
}
a();
```

##### Results
This python code, took about 3.5 seconds to run.\
My ship code, took about 3.1 seconds to run. (Without a GC);

