# Заданиe 2. Работа с mdadm.

## Создание дисков.
Создал 5 одинаковый дисков через интерфейс UTM.

    student@Otus:~$ sudo lshw -short | grep disk
    /0/100/5/0.0.0         /dev/sda   disk           1073MB QEMU HARDDISK
    /0/100/5/0.1.0         /dev/sdb   disk           1073MB QEMU HARDDISK
    /0/100/5/0.2.0         /dev/sdc   disk           1073MB QEMU HARDDISK
    /0/100/5/0.3.0         /dev/sdd   disk           1073MB QEMU HARDDISK
    /0/100/5/0.4.0         /dev/sde   disk           1073MB QEMU HARDDISK
    /0/100/1f.2/0.0.0      /dev/sdf   disk           32GB QEMU HARDDISK

## Создание массива.

    student@Otus:~$ sudo mdadm --create --verbose /dev/md0 -l 5 -n 5 /dev/sd{a,b,c,d,e}
    mdadm: layout defaults to left-symmetric
    mdadm: layout defaults to left-symmetric
    mdadm: chunk size defaults to 512K
    mdadm: size set to 1046528K
    mdadm: Defaulting to version 1.2 metadata
    mdadm: array /dev/md0 started.

    student@Otus:~$ cat /proc/mdstat
    Personalities : [raid6] [raid5] [raid4] 
    md0 : active raid5 sde[5] sdd[3] sdc[2] sdb[1] sda[0]
          4186112 blocks super 1.2 level 5, 512k chunk, algorithm 2 [5/5] [UUUUU]
          
    unused devices: <none>

    student@Otus:~$ sudo mdadm -D /dev/md0
    /dev/md0:
               Version : 1.2
         Creation Time : Wed Feb 25 06:36:14 2026
            Raid Level : raid5
            Array Size : 4186112 (3.99 GiB 4.29 GB)
         Used Dev Size : 1046528 (1022.00 MiB 1071.64 MB)
          Raid Devices : 5
         Total Devices : 5
           Persistence : Superblock is persistent
    
           Update Time : Wed Feb 25 06:36:24 2026
                 State : clean 
        Active Devices : 5
       Working Devices : 5
        Failed Devices : 0
         Spare Devices : 0
    
                Layout : left-symmetric
            Chunk Size : 512K
    
    Consistency Policy : resync
    
                  Name : Otus:0  (local to host Otus)
                  UUID : ff708ac4:771c4448:2f06144b:14170a90
                Events : 19
    
        Number   Major   Minor   RaidDevice State
           0       8        0        0      active sync   /dev/sda
           1       8       16        1      active sync   /dev/sdb
           2       8       32        2      active sync   /dev/sdc
           3       8       48        3      active sync   /dev/sdd
           5       8       64        4      active sync   /dev/sde

# Проверка поломки.

Фейлим и удаляем диск из массива.

    student@Otus:~$ sudo mdadm /dev/md0 --fail /dev/sdb
    mdadm: set /dev/sdb faulty in /dev/md0

    student@Otus:~$ cat /proc/mdstat
    Personalities : [raid6] [raid5] [raid4] 
    md0 : active raid5 sde[5] sdd[3] sdc[2] sdb[1](F) sda[0]
          4186112 blocks super 1.2 level 5, 512k chunk, algorithm 2 [5/4] [U_UUU]
          
    unused devices: <none>
    
    student@Otus:~$ sudo mdadm /dev/md0 --remove /dev/sdb
    mdadm: hot removed /dev/sdb from /dev/md0

Возвращаем диск обратно.
    
    student@Otus:~$ sudo mdadm /dev/md0 --add /dev/sdb
    mdadm: added /dev/sdb

    student@Otus:~$ cat /proc/mdstat
    Personalities : [raid6] [raid5] [raid4] 
    md0 : active raid5 sdb[6] sde[5] sdd[3] sdc[2] sda[0]
          4186112 blocks super 1.2 level 5, 512k chunk, algorithm 2 [5/5] [UUUUU]
          
    unused devices: <none>

# Разделы и файловая система.

    student@Otus:~$ sudo parted -s /dev/md0 mklabel gpt
    student@Otus:~$ sudo parted /dev/md0 mkpart primary ext4 0% 20%
    Information: You may need to update /etc/fstab.
    
    student@Otus:~$ sudo parted /dev/md0 mkpart primary ext4 20% 40%
    Information: You may need to update /etc/fstab.
    
    student@Otus:~$ sudo parted /dev/md0 mkpart primary ext4 40% 60%          
    Information: You may need to update /etc/fstab.
    
    student@Otus:~$ sudo parted /dev/md0 mkpart primary ext4 60% 80%          
    Information: You may need to update /etc/fstab.
    
    student@Otus:~$ sudo parted /dev/md0 mkpart primary ext4 80% 100%         
    Information: You may need to update /etc/fstab.
    
    student@Otus:~$ for i in $(seq 1 5); do sudo mkfs.ext4 /dev/md0p$i; done  
    mke2fs 1.47.0 (5-Feb-2023)
    Creating filesystem with 208896 4k blocks and 52304 inodes
    Filesystem UUID: 149f2380-5660-4db5-adff-7e26e1c60c4b
    Superblock backups stored on blocks: 
    	32768, 98304, 163840
    
    Allocating group tables: done                            
    Writing inode tables: done                            
    Creating journal (4096 blocks): done
    Writing superblocks and filesystem accounting information: done
    
    mke2fs 1.47.0 (5-Feb-2023)
    Creating filesystem with 209408 4k blocks and 52416 inodes
    Filesystem UUID: 06488901-0cbf-4c40-a985-cf894ebee68b
    Superblock backups stored on blocks: 
    	32768, 98304, 163840
    
    Allocating group tables: done                            
    Writing inode tables: done                            
    Creating journal (4096 blocks): done
    Writing superblocks and filesystem accounting information: done
    
    mke2fs 1.47.0 (5-Feb-2023)
    Creating filesystem with 208896 4k blocks and 52304 inodes
    Filesystem UUID: 4b255483-e3be-4591-aaec-def6473b8d40
    Superblock backups stored on blocks: 
    	32768, 98304, 163840
    
    Allocating group tables: done                            
    Writing inode tables: done                            
    Creating journal (4096 blocks): done
    Writing superblocks and filesystem accounting information: done
    
    mke2fs 1.47.0 (5-Feb-2023)
    Creating filesystem with 209408 4k blocks and 52416 inodes
    Filesystem UUID: 1e0dd398-f4da-4df6-aae7-4cb30c61d057
    Superblock backups stored on blocks: 
    	32768, 98304, 163840
    
    Allocating group tables: done                            
    Writing inode tables: done                            
    Creating journal (4096 blocks): done
    Writing superblocks and filesystem accounting information: done
    
    mke2fs 1.47.0 (5-Feb-2023)
    Creating filesystem with 208896 4k blocks and 52304 inodes
    Filesystem UUID: 72fda310-5953-41e7-acba-e96a37d96964
    Superblock backups stored on blocks: 
    	32768, 98304, 163840
    
    Allocating group tables: done                            
    Writing inode tables: done                            
    Creating journal (4096 blocks): done
    Writing superblocks and filesystem accounting information: done
    
    student@Otus:~$ sudo mkdir -p /raid/part{1,2,3,4,5}
    student@Otus:~$ for i in $(seq 1 5); do sudo mount /dev/md0p$i /raid/part$i; done
    student@Otus:~$ ls /raid/part
    part1/ part2/ part3/ part4/ part5/ 
    student@Otus:~$ ls /raid/part1/
    lost+found
