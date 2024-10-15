# Motivation

Originally I created my first cross thread channel for my Computer networks class when I was
an undergraduate at Utah State. We built a simple TCP/IP stack on top of raw sockets in the
Linux stack, and we were required to have our socket listening on one thread, and then to
have received packets (ARP, IP, etc.) processed on their own threads. The assignment came
with a default cross-thread channel we could use, but it was essentially C code that had been
lightly ported to C++. It was also locked to using PThreads, but I realized it might be nice to
be able to use the channels in future projects on Windows or Mac as well. That's when I
started making Narrows.

## Inspiration

I used the `crossbeam` crate in Rust a few years ago when I completed a course on the Rust
programming language. I liked the ease of use of the package, and I particularly like the
syntax for generating a new channel:

```rust
use crossbeam_channel::unbounded;

let (s, r) = unbounded();

s.send(0).unwrap();
assert_eq!(r.recv(), Ok(0));
```

I also liked the type safety of the `crossbeam` channel. Once you create a channel of type `T` you can only
send `T`'s across the channel. I think that the tradeoff I was willing to make there was one where
I could sacrifice the potential performance of having one channel that accepted multiple types in
exchange for the ease of reasoning that came from having multiple channels that each had their own
unique type.

When prototyping out ideas I also found andreiavrammsd/cpp-channel which is a great implementation
of a cross-thread channel that did most of what I wanted. The goal of this library is to expand
`cpp-channel` and to flavor with a bit of my own seasoning.

## Design

Based off `crossbeam` and `cpp-channel` I decided that I wanted to support four general types of
channels, with each sharing a common interface and operations. The four channels are categorized
as *bounded* or *unbounded* channels, and *single* or *multi* channels as follows:

|         | Bounded-      | Unbounded-    |
|---------| ------------- | ------------- |
| Single- | `nrws::bounded_single<T>`  | `nrws::unbounded_single<T>`  |
| Multi-  | `nrws::bounded_multi<T>`  | `nrws::unbounded_multi<T>`  |

(`nrws::` is the namespace for narrows).

| Channel Type | Description |
|    :---:     |    :---:    |
| Bounded | Only a fixed number of items can be stored in the channel at once |
| Unbounded | The channel can grow as long as it has memory |
| Single | There is a single producer and a single consumer for the channel |
| Multi | There can be $n$ producers and $m$ consumers on the channel, $m,n\in \mathbb{N}$ |
