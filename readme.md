
```shell
C:\"Program Files"\qemu\qemu-system-x86_64.exe -m 32M -boot c -drive file=Z:\home\rhett\Onix\build\master.img,format=raw,index=0,media=disk,if=ide -audiodev driver=sdl,id=hda -machine pcspk-audiodev=hda -rtc base=localtime

bochsdbg -q -f bochsrc.bxrc
```
