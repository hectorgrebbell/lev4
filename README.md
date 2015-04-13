# Level 4 work
A few snippets of code from 4th year of university.

Aiming to add something OO (C++, C# or Java) & something from my
dissertation project (adaptive media streaming over HTTP testbed -
C++, libpcap, cross platform - x64 & ARM) shortly.

* **sac16-cracker** contains an exert from some cybersecurity coursework
  implemented in C. We were given a ciphertext & compiled java classes
  to perform the encryption & decryption. We were expected to perform
  a brute force on the keyspace & use language recognition to identify
  the correct key. This was implemented due to some lecturers tendency
  to mark exactly to a mark scheme, but I also decompiled the provided
  code & found weaknesses in the encryption algorithm to allow the key to
  be identified in far less time. The interesting code is in [CTO.c]
  (https://github.com/hectorgrebbell/lev4/blob/master/sac16-cracker/CTO.c).
  Frequency data was calculated from a selection of english texts from
  [Project Gutenburg](https://www.gutenberg.org/).
  
* **sound-classifier** is from some artificial intelligence coursework
  implemented in C. Its designed to show knowledge of K-fold validation
  and Naive Bayers classification. It reads in 100 files representing
  recordings of either speech or silence. It trains with 90 of the files
  then classifies the remaining 10. The interesting code is in [classifier.c]
  (https://github.com/hectorgrebbell/lev4/blob/master/sound-classifier/classifier.c)
  The code was implemented in C & uses the horrendous mess of pointer
  arithmatic mostly to win a bet with a friend that I could run on the
  supplied sample data in under a 30th of a second on a university
  machine.
  
* **tem_conversion** contains a pretty triavial/pointless bit of Erlang
  completed for an Advanced Operating systems course to show knowledge
  of message passing. I think Erlang manages to be simultaneously the
  nicest & most horrendous language i've ever used.
