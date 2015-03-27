# `procedure.md`

## Platform

`uname -a` outputs:

```
Linux os2 3.14.36-1-lts #1 SMP Wed Mar 18 18:15:02 CET 2015 x86_64 GNU/Linux
```

This is an ArchLinux VM running on VirtualBox 4.3.20.

The hardware is a Mid-2011 MacBook Air with
- 4 GB 1333 Mhz DDR3 RAM
- 1.7 GHz Intel Core i5 (four hardware threads)

## Methodology

Run `httperf` with the following parameters:
- 10 mb file
- single port
- 5000 connections

Modulate the rate each time, with the following rates tested:
- 50 request per second
- 100 " "
- 125 " "
- 150 " "
- 175 " "
- 200 " "

Test on each of `http-server`, `lcrq-server`, and `msq`.

For each server, 1000 requests at a rate of 100 are sent as warmup.

