# BM1387-miner-snippets
Code snippets for BM1387 miner

(I'm using as little as possible libraries, and try to avoid any dynamic memory allocations, so no String type variables, classes etc. This to prevent memory leaks and code bulging. This does mean that most code is for single threaded use only!)

All code is for reference only and still under construction/untested except:

poolio.h - functional but still limited & blocking on connect

bm1387freq.ino - fully tested but still contains tons of debug printfs
<br>
<br>
My mining software state:<br>
<br>
Right after startup:<br>
<code>056fde77001b92 73972f049e5ab64237757e4ffd5a14721bbdc9f2ba27162c137f0aba9054c8e6
0c3766d0011f80 cdd6788b2a376a01d5fda247fc145c75a757655ec465e935e14019f0024438dc
82fb56f8000785 71ee60300d26bf68e6fe05443fed779ca657065e57f855d7bacc9ae60a7a6d15

-----------------------------------------------------------------------------------
Hashrate (target:27.93GHs): 25.42GHs
asic#0:12.95GHs asic#1:8.80GHs 
Job interval (ms): 19 Vcore:0.66 Status: -warming up- -no pool data-
-----------------------------------------------------------------------------------
10772075010991 f5f7e1b3aa8373159d1938c8832f2a6e2beffef8b09c5e0b6ff150ec0cc6cf08
0a695dd2010b81 7bb935b29384b03edaca4c0d0458cdb628aca5e4ef6d4b6bffc86a6eabb552db
0c1c6d17010298 61879e7a0160ff774b87d55a7d4cdec6c37fa40d6cfb399f40d13bde516fe130
</code>
<br>
and after a minute:<br>
<br>
<code>a8cff0a100279f 773b72dec5ea5a5592d3bc77a0dbd524559cd1953172866b816fe9a900000000
87fcbcc2002986 ed163ea5416dd25fc3c36e0bdd3a49cb927ffba337754a76b567b38900000000
c70d280400308f 669a634bf0ff9a8499a476ff13a89d6151a268621f2434f0fa54c2a400000000
8bf0d4b700339c 554d340cc905da9ea662d2f6513fffece5373201138eae4fbebb2cab00000000

-----------------------------------------------------------------------------------
Hashrate (target:27.93GHs): 26.04GHs
asic#0:13.21GHs asic#1:12.13GHs 
Job interval (ms): 76 Vcore:0.66 Status: -mining- -pool active-
-----------------------------------------------------------------------------------
1d99ee22003690 ef21d5a1b1e37eb93660083bd6b52607ccf543217dd7effdf3c73ae000000000
0ac3f4d1003894 711b888e8f47467ba20aca0a6653b2d2afcf832d2500f805a7cde46200000000
3124b686003894 2eb550e8ae7c1712ed268be37e87c644b139f3b1868c163486cdf66000000000
42f91e57023b85 79b77d746237d62b113f744353fa7a345ce65c99d8455cf624e8f7c100000000
</code><br>
During warmup the asics get fed random data, this activates all cores in the asics. The Antminer S9 does something similar during warmup. If I don't use a warmup my hashrate is significantly lower..<br>The asics run at 120MHz(asic#0) and 125MHz(asic#1), and will in time each reach the target hashrate.<br>
The 1st hex block shows the tickets (nonces) returned by the asics, the second block the final (not yet reversed) hash. During mining stage it ends with at least 8 zeroes, success!!
