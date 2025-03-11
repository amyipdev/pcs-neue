# pcs-neue

A new proactive compaction strategy for the Linux kernel targeted at periodic-allocating applications.

## Running the Tools

### Prerequisites

All you need is a recent Nix implementation with `nix-command` and `flakes` enabled. 
If you don't have these enabled, check out [Xe Iaso](https://xeiaso.net/blog/nix-flakes-1-2022-02-21/)'s guide.

If you don't have Nix already, you should try out [Lix](https://lix.systems/). 

Make sure to download and enter the repo:

```sh
# if you don't have SSH set up
git clone https://github.com/amyipdev/pcs-neue
# if you do have SSH set up
git clone git@github.com:amyipdev/pcs-neue
cd pcs-neue
```

### Buddyinfo

Buddyinfo is our buddy allocator visualization tool. You can build it with

```sh
nix build .\#buddyinfo
cp result buddyinfo-bin
```

You should then choose a refresh frequency, between 0.05s and 1s, and run `watch` with it (0.1 as an example):

```sh
watch -n 0.1 ./buddyinfo-bin
```

### PCSbench

PCSbench is our benchmarking tool. You can build it with

```sh
nix build .\#pcsbench
cp result pcsbench-bin
```

You can then run `./pcsbench` to run the default test, or `./pcsbench --bench=N` to run test N.
