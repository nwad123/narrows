> [!WARNING]
> This is very much a work in progress and is not ready for use quite yet.

<div align="center">
  <img src="https://github.com/nwad123/narrows/blob/main/doc/img/NarrowsGemini.png" width=300>

  <h1>
    Narrows
  </h1>
</div>

Narrows is a C++ channel library providing efficient and easy-to-use communication channels for concurrent programming.
This library was a chance for me to learn more personally about cross thread channels, and you probably want to make sure
you consider the tradeoffs I've made before using it for your project. Once I'm done with it, this README will give a good
introduction to narrows, but for more information I'd suggest poking around the [docs](./doc/) folder, as it has some writing 
on the history and design of narrows and can give you an idea of where the library came from.

## Features

- Bounded channel: A channel with a fixed capacity
    
    ```cpp
    auto [s, r] = nrws::bounded<int>(10);
    ```

- Unbounded channel: A channel with no upper bound on its capacity
    
    ```cpp
    auto [s, r] = nrws::unbounded<int>();
    ```

## Design Tradeoffs

- Bounded channel
  - [Add tradeoffs here]
- Unbounded channel
  - [Add tradeoffs here]

## Installation

[Provide installation instructions here]

## Usage

### Bounded Channel

```cpp
// Example code for using a bounded channel
```

### Unbounded Channel

```cpp
// Example code for using an unbounded channel
```

## API Reference

[Provide a brief overview of key classes and functions]

## Performance

[Include any performance metrics or comparisons]

## Contributing

[Instructions for contributors]

## License

[Specify the license]

## Acknowledgements

- Inspired by [andreiavrammsd/cpp-channel](https://github.com/andreiavrammsd/cpp-channel)
- Inspired by Rust's Crossbeam library
- [Any other acknowledgements]
