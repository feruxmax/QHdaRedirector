# QHdaRedirector
Intel HDA ports remapping tool for linux

sudo rights are required for access to sound port and settings storing

"Write settings" button will result in writing config to 
```/lib/firmware/snd-hda-intel.fw```
and enabling it by writing to 
```/etc/modprobe.d/hda-intel.conf```

[Related information](https://www.kernel.org/doc/html/v4.12/sound/hd-audio/notes.html)
