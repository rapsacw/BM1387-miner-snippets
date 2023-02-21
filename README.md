# BM1387-miner-snippets
Code snippets for BM1387 miner

(I'm using as little as possible libraries, and try to avoid any dynamic memory allocations, so no String type variables, classes etc. This to prevent memory leaks and code bulging. This does mean that most code is for single threaded use only!)

All code is for reference only and still under construction/untested except:

poolio.h - functional but still limited & blocking on connect

bm1387freq.ino - fully tested but still contains tons of debug printfs

