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
| Multi | There can be $n$ producers and $m$ consumers on the channel |

### Construction

I also wanted all of the channels to have the same construction interface, as well as similar
operating interfaces. Ideally, this would make it possible to switch from any type of channel
to another, with minimal changes in the code (with the exception of changing from `multi` to
`single`, as that would likely require logical changes).

```cpp
// General construction interface
auto [sender, receiver] = nrws::channel_type::new<T>();
```

If we were to model this as a C++20 concept, it might look something like this (note that this
is uncompiled and tested code, I'll update it in the future).

```cpp
template<typename T, typename Sender, typename Receiver, typename Channel>
concept is_channel = requires {
    { Channel::new() } -> std::convertible_to<std::pair<Sender, Receiver>>;
};
```

From here we can see that we need some way to ensure that the `Sender` and `Receiver` type
properly conform to some sort of interface, so we could also define concepts for those.

### Aside on Convention Choice

I want to note that I originally did *not* structure my channels like this. My original
implementation was a templated channel that acted as a sender and receiver, and could be
passed around by reference to be used by any thread. This lead to some concerns though as I
started to grow my TCP/IP application for class. Who owned the channel? Should it be the
main thread? The socket thread? How did I differentitate between messages sent from one
thread to another? If the channel goes out of scope earlier than planned (maybe an error
occurs in the socket code or something), then how do we handle the references to the channel?

These questions then lead me to more library design questions that I personally didn't
have issues with on my school assignment, but I imagined could be issues in the future. Say
I'm working in a large codebase in the future with multiple people. What happens if the channel
is used to send *and* receive on the same thread? That may be desirable at times, for example
if we receive an ARP packet we may want to send an response to it. To accomplish this we could
send the ARP packet over the channel to the ARP processing thread, then wait for the ARP
processing thread to send us back a response. While this would work, I wanted a safer design
that had more guarantees about how it functioned, and one that was clear from it's API who
owned the channel and how the channel could be used.

### Sender/Reciever Concepts

A sender needs to do one basic operation: *send*. By the same token, a receiver needs to, you
guessed it, *receive* a value. However, let's consider error handling too. My original three
or four implementations had very little error handling, which was silly because this is an
operation that is error prone. Here are some of the most common errors we can get with the
*send* and *receive* operations:

| Error | Explanation |
| :---: | :---- |
| `sender_error::closed_channel` | Any item cannot be sent on a closed channel, so this is an error |
| `sender_error::channel_full` | For bounded channels there is a limit to how many things can be sent without being received |
| `receiver_error::empty_channel` | You can't receive something on an empty channel

There are other possible errors I'm sure I'll run into, but that's what I have found for
now. Since these cases are fairly possible to happen to anyone, I'll make sure that my code
handles them well. To do so I'll be using `std::expected`, at least until it makes me sad and
I have to code up my own variant of it.

```cpp
template<typename Sender, typename T>
concept is_sender = requires(Sender<T> s, T val){
    { s.send(val) } -> std::convertible_to<std::expected<void, sender_error>>;
};
```

```cpp
template<typename Receiver, typename T>
concept is_receiver = requires(Receiver<T> r){
    { r.recv() } -> std::convertible_to<std::expected<T, receiver_error>>;
};
```

### Tradeoffs

I think that this design leaves things pretty open to the implementer of the interfaces, me. A
general specification for *bounded* vs. *unbounded* and *single* vs. *multi* channels was
developed and a simple interface for construction and senders/receivers was made. I made these
design choices based off the three or four initial implementations I made, and I think they
will work well for my use cases now and in the future.

Tradeoffs have been discussed through this doc, and ultimately I'll make more mention of
tradeoffs as I develop this library more fully.
