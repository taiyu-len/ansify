ANSIFY
======

turns images into simple ascii art.

usages
------
```
./ansify -k nn:nn:nn -t 1 -d 10000 image1 image2 image3 ... imagen > file
```
-k chooses the transparent color key. (default is none)

-t sets the threshold (how close to the colorkey before its made
transparents) (default is 10)

-d sets line print delay in microseconds.


trimming
--------
for useless space you can pass input to
  ```
  sed "s, +\(\o33\[0m\),\1,"
  ```
  to get rid of it.

