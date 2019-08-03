cpuminer-Xen is a fork of cpuminer-opt & cpuminer-multi by TPruvot with optimizations
imported from other miners developped by lucas Jones, djm34, Wolf0, pooler,
Jeff garzik, ig0tik3d, elmad, palmd, barrystyle, Jay D Dee and Optiminer, with additional
optimizations by RickillerZ & XEnom.

See file RELEASE_NOTES for change log and INSTALL_LINUX or INSTALL_WINDOWS
for compile instructions.

Requirements
------------

1. A x86_64 architecture CPU with a minimum of SSE2 support. This includes
Intel Core2 and newer and AMD equivalents. In order to take advantage of AES_NI
optimizations a CPU with AES_NI is required. This includes Intel Westbridge
and newer and AMD equivalents. Further optimizations are available on some
algoritms for CPUs with AVX and AVX2, Sandybridge and Haswell respectively.

Older CPUs are supported by cpuminer-multi by TPruvot but at reduced
performance.

ARM CPUs are not supported.

2. 64 bit Linux OS. Ubuntu and Fedora based distributions, including Mint and
Centos, are known to work and have all dependencies in their repositories.
Others may work but may require more effort. Older versions such as Centos 6
don't work due to missing features. 
64 bit Windows OS is supported with mingw_w64 and msys or pre-built binaries.

MacOS, OSx and Android are not supported.

3. Stratum pool. Some algos may work wallet mining using getwork or GBT. YMMV.

Supported Algorithms
--------------------

                          allium        Garlicoin
                          anime         Animecoin
                          argon2        Argon2 coin (AR2)
                          argon2d250    argon2d-crds, Credits (CRDS)
                          argon2d500    argon2d-dyn,  Dynamic (DYN)
                          argon2d4096   argon2d-uis, Unitus, (UIS)
                          axiom         Shabal-256 MemoHash
                          bastion
                          blake         Blake-256 (SFR)
                          blakecoin     blake256r8
                          blake2s       Blake-2 S
                          bmw           BMW 256
                          c11           Chaincoin
                          decred
                          deep          Deepcoin (DCN)
                          dmd-gr        Diamond-Groestl
                          drop          Dropcoin
                          fresh         Fresh
                          groestl       Groestl coin
                          heavy         Heavy
                          hmq1725       Espers
                          hodl          Hodlcoin
                          jha           Jackpotcoin
                          keccak        Maxcoin
                          keccakc       Creative coin
                          lbry          LBC, LBRY Credits
                          luffa         Luffa
                          lyra2h        Hppcoin
                          lyra2re       lyra2
                          lyra2rev2     lyra2v2
                          lyra2rev3     lyrav2v3, Vertcoin
                          lyra2z        
                          lyra2z330     Lyra2 330 rows, Zoin (ZOI)
                          m7m           Magi (XMG)
                          myr-gr        Myriad-Groestl
                          neoscrypt     NeoScrypt(128, 2, 1)
                          nist5         Nist5
                          pentablake    Pentablake
                          phi1612       phi, LUX coin (original algo)
                          phi2          LUX coin (new algo)
                          pluck         Pluck:128 (Supcoin)
                          polytimos     Ninja
                          quark         Quark
                          qubit         Qubit
                          scrypt        scrypt(1024, 1, 1) (default)
                          scrypt:N      scrypt(N, 1, 1)
                          scryptjane:nf
                          sha256d       Double SHA-256
                          sha256q       Quad SHA-256, Pyrite (PYE)
                          sha256t       Triple SHA-256, Onecoin (OC)
                          shavite3      Shavite3
                          skein         Skein+Sha (Skeincoin)
                          skein2        Double Skein (Woodcoin)
                          skunk         Signatum (SIGT)
                          sonoa         Sono
                          timetravel    Machinecoin (MAC)
                          timetravel10  Bitcore
                          tribus        Denarius (DNR)
                          vanilla       blake256r8vnl (VCash)
                          veltor        (VLT)
                          whirlpool
                          whirlpoolx
                          x11           Dash
                          x11evo        Revolvercoin
                          x11gost       sib (SibCoin)
                          x12           Galaxie Cash (GCH)
                          x13           X13
                          x13sm3        hsr (Hshare)
                          x14           X14
                          x15           X15
                          x16r          Ravencoin (RVN)
                          x16s          pigeoncoin (PGN)
                          x17
                          xevan         Bitsend (BSD)
                          yescrypt      Globalboost-Y (BSTY)
                          yescryptr8    BitZeny (ZNY)
                          yescryptr16   Eli
                          yescryptr32   WAVI
                          yespower      Cryply
                          yespowerr16   Yenten (YTN)
                          yespowerurx   Yespower (URX)
                          yespowerltncg Yespower (LTCNG)
                          cpupower      CPUchain (CPU)
                          zr5           Ziftr

						  
![](https://i.imgur.com/CQnZOuA.gif)


Donations
---------

cpuminer-Xen has no fees of any kind but donations are accepted.

 BTC: 3LvzNGmiko44yZ4MzFFX2tneEg4yRMXhZ7 | LTC: MSYzi5zSCwEJsPy7FVDQiUsZkK9iJW3Mxz | DOGE DNrWJASoLM54UvBVZhX1qd99sCYW9ZADUo | CPU: Cb1j2TivwGky5iBanoeVjZ47GkUSCUR4pN | LTCNG: MCTDcQ87pDnY4qCjLn1gk8FLmSWdWLw9aD

Happy mining!

