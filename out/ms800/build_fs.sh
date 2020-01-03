
mkfs.jffs2 -e 0x8000 -d ./system_chird -o ./system_chird.jffs2
mkcramfs ./system_chird/  ./system_chird.cramfs
#mkfs.jffs2 -e 0x8000 -p 0x650000 -d ./data_chird -o ./data_chird.jffs2

