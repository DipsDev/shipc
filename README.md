# Ship the programming language, once again!

I wanted to create a fully blown programming language and learn a bit of C.\
The language is 100% custom-made (only std libraries), with its own VM and bytecode version.\
\
Ship is a dynamic, high-level, garbage-collected language inspired by Javascript and Lua.\
Ship uses curly braces (`{}`) to identify blocks, and requires `;`.


## Features
### Values
Ship has 4 types of values.
number, boolean, nil and object.\
strings, functions, arrays and classes are considered objects.


#### Arrays
Arrays are mutable, reference related objects. which start at index 0.
```javascript
var a = [1, 2, 3, 5]
var b = a;
b.push(100); // will affect a too!
```

| Attribute     | Arguments | Description                                     | Return Type  |
|---------------|-----------|-------------------------------------------------|--------------|
| Array.push(v) | v: Any    | Pushes args[0] into the last index of the array | Nil          |
| Array.pop(n)  | n: Number | Pops n elements from the end of the array       | Array or Any |
| Array.len()   |           | Returns the length of an array.                 | Number       |

#### Strings
Strings are made using `"`.
```javascript
var x = "Hello World"; // Simple String
```
| Attribute      | Arguments | Description                                | Return Type                     |
|----------------|-----------|--------------------------------------------|---------------------------------|
| String.len()   |           | Returns the length of the string           | Number                          |
| String.title() |           | Capitalizes the first letter of the string | String. a copy of the original. |
| String.copy()  |           | Returns a copy of the given string         | String                          |


#### Numbers
Numbers are doubles.
```javascript
var x = 4.0;
var x = 3;
```


| Attribute       | Arguments | Description                                           | Return Type |
|-----------------|-----------|-------------------------------------------------------|-------------|
| Number.to_str() |           | Returns the number as string                          | String      |
| Number.next()   |           | Returns the next consecutive integer. (n + 1)         | Number      |
| Number.pred()   |           | Returns the previous consecutive integer. (n - 1)     | Number      |
| Number.times()  |           | Returns an array containing [0, self) excluding       | Array       |
| Number.odd()    |           | Returns whether or not the number is odd              | Boolean     |
| Number.even()   |           | Returns whether or not the number is even             | Boolean     |
| Number.upto(n)  | n: Number | Returns an array from self until n [self, n] included | Array       |

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
ship has a great error information. and will try to guide you through the code.\
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
Ship helps developers by using a robust error system, to help developers identify common errors.
```
error: Missing parentheses in call
    [main.ship:8:16]
    |
  8 | print(get_max100, 34));
    |             ^^^^
```

## Running
Ship uses cmake to handle builds.\
In order to create a release build by yourself, build the cmake project in release mode.
```
$ cmake -DCMAKE_BUILD_TYPE=Release -S /path/to/source-dir -B /path/to/build-dir
$ cmake --build /path/to/build-dir
```

## Roadmap
- While loops (Done)
- Global and local variables (Done)
- Math expressions (Done)
- Functions (Done)
- Function arguments (Done)
- Garbage collector (Done ish)
