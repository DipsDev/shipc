# Ship the programming language, once again!

#### This is probably my last implementation of ship. but now in C.

This project served as a learning project to learn the C language.\
I might have made some mistakes here and there, and I will continue to improve on this language.\
I will expand the language features here, once I finish most.\

The language is 100% custom-made, with its own VM and bytecode version.


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

