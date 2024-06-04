# Ship the programming language, once again!

#### This is probably my last implementation of ship. but now in C.

This project served as a learning project for the C language.\
I might have made some mistakes here and there, and I will continue to improve on this language.\

The language is 100% custom-made, with its own VM and bytecode version.\
Ship is a dynamic, high-level garbage-collected language inspired by javascript and lua.\
Ship uses curly braces (`{}`) to identify blocks, and signifies end of statements with `;`


## Features

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

Foreach loops are a different thing,\
ship supports the following syntax similar to how for loop works in python.\
the following ship code equals to the following python code:\
`main.ship`
```ruby
var best_lang = "ship";
foreach best_lang |char| {
    print(char);
}
```
`main.py`
```python
best_lang = "ship"
for char in best_lang:
    print(char)
```



### Conditionals
Ship uses a go like syntax in order to create if statements.
```javascript
var a = 15;
if a >= 3 {
    ...
}
```

### Arrays
Arrays are a new feature in ship. they work like any other language.
```rust
var classes = ["Biology", "Math", "Music", "English"];
foreach classes |class| {
    print(class);
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
```javascript
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
- Garbage collector (Done ish)



## More Information

### Performance

Consider the following python code:
```python
def a():
    x = 0
    while x <= 1_000_000_000:
        x = x - 1
a()
```

And it's .ship version:
```rust
fn a() {
    var x = 0;
    while x <= 1_000_000_000 {
        x = x - 1;
    }
}
a();
```

##### Results
This python code, took about 32.3 seconds to run.\
My ship code, took about 28.8 seconds to run. 

