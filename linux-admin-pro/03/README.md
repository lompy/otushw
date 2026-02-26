# Создание логического раздела.

    root@Otus:~# lsblk
    NAME                MAJ:MIN RM  SIZE RO TYPE MOUNTPOINTS
    sda                   8:0    0    1G  0 disk
    sdb                   8:16   0   10G  0 disk
    sdc                   8:32   0    2G  0 disk
    sdd                   8:48   0    1G  0 disk
    sde                   8:64   0   30G  0 disk
    |-sde1                8:65   0  487M  0 part /boot
    |-sde2                8:66   0    1K  0 part
    `-sde5                8:69   0 29.5G  0 part
      |-Otus--vg-root   254:0    0 28.6G  0 lvm  /
      `-Otus--vg-swap_1 254:1    0  976M  0 lvm  [SWAP]
    sdf                   8:80   0    1G  0 disk

    root@Otus:~# pvcreate /dev/sdb
      Physical volume "/dev/sdb" successfully created.

    root@Otus:~# vgcreate otus /dev/sdb
      Volume group "otus" successfully created

    root@Otus:~# lvcreate -l+80%FREE -n test otus
      Logical volume "test" created.

    root@Otus:~# vgdisplay otus
      --- Volume group ---
      VG Name               otus
      System ID
      Format                lvm2
      Metadata Areas        1
      Metadata Sequence No  2
      VG Access             read/write
      VG Status             resizable
      MAX LV                0
      Cur LV                1
      Open LV               0
      Max PV                0
      Cur PV                1
      Act PV                1
      VG Size               <10.00 GiB
      PE Size               4.00 MiB
      Total PE              2559
      Alloc PE / Size       2047 / <8.00 GiB
      Free  PE / Size       512 / 2.00 GiB
      VG UUID               a1Q2tV-1bUE-v43K-qEZz-zd5H-FadT-L1z84O

    root@Otus:~# vgdisplay -v otus | grep 'PV Name'
      PV Name               /dev/sdb

    root@Otus:~# lvdisplay /dev/otus/test
      --- Logical volume ---
      LV Path                /dev/otus/test
      LV Name                test
      VG Name                otus
      LV UUID                Jo5FLx-DtJ9-R4jx-4fGh-LNgU-oW51-gudmW1
      LV Write Access        read/write
      LV Creation host, time Otus, 2026-02-25 14:38:03 +0300
      LV Status              available
      # open                 0
      LV Size                <8.00 GiB
      Current LE             2047
      Segments               1
      Allocation             inherit
      Read ahead sectors     auto
      - currently set to     256
      Block device           254:2

# Создание логического раздела на свободном месте с монтированием.

    root@Otus:~# lvcreate -L100M -n small otus
      Logical volume "small" created.

    root@Otus:~# lvs
      LV     VG      Attr       LSize   Pool Origin Data%  Meta%  Move Log Cpy%Sync Convert
      root   Otus-vg -wi-ao----  28.56g
      swap_1 Otus-vg -wi-ao---- 976.00m
      small  otus    -wi-a----- 100.00m
      test   otus    -wi-a-----  <8.00g

    root@Otus:~# mkfs.ext4 /dev/otus/test
    mke2fs 1.47.0 (5-Feb-2023)
    Discarding device blocks: done
    Creating filesystem with 2096128 4k blocks and 524288 inodes
    Filesystem UUID: bb6cc1d1-faf8-4592-85d0-c9509297f38c
    Superblock backups stored on blocks:
        32768, 98304, 163840, 229376, 294912, 819200, 884736, 1605632

    Allocating group tables: done
    Writing inode tables: done
    Creating journal (16384 blocks): done
    Writing superblocks and filesystem accounting information: done

    root@Otus:~# mkdir /data
    root@Otus:~# mount /dev/otus/test /data/
    root@Otus:~# mount | grep /data
    /dev/mapper/otus-test on /data type ext4 (rw,relatime)

# Уменьшить том под / до NGB.

Зачистим стенд.

    root@Otus:~# vgrename otus vg_root
      Volume group "otus" successfully renamed to "vg_root"

    root@Otus:~# lvrename vg_root/test vg_root/lv_root
      Renamed "test" to "lv_root" in volume group "vg_root"

    root@Otus:~# lvremove vg_root/small
    Do you really want to remove active logical volume vg_root/small? [y/n]: y
      Logical volume "small" successfully removed.

    root@Otus:~# lvdisplay
      --- Logical volume ---
      LV Path                /dev/Otus-vg/root
      LV Name                root
      VG Name                Otus-vg
      LV UUID                6W5ndR-AlbR-dDzn-8F50-LzF9-DT52-2TUiiu
      LV Write Access        read/write
      LV Creation host, time Otus, 2025-03-12 16:55:22 +0300
      LV Status              available
      # open                 1
      LV Size                28.56 GiB
      Current LE             7312
      Segments               1
      Allocation             inherit
      Read ahead sectors     auto
      - currently set to     256
      Block device           254:0

      --- Logical volume ---
      LV Path                /dev/Otus-vg/swap_1
      LV Name                swap_1
      VG Name                Otus-vg
      LV UUID                JTG5ne-XboE-F2bY-WSqv-8fPc-SbzI-BSwJGC
      LV Write Access        read/write
      LV Creation host, time Otus, 2025-03-12 16:55:23 +0300
      LV Status              available
      # open                 2
      LV Size                976.00 MiB
      Current LE             244
      Segments               1
      Allocation             inherit
      Read ahead sectors     auto
      - currently set to     256
      Block device           254:1

      --- Logical volume ---
      LV Path                /dev/vg_root/lv_root
      LV Name                lv_root
      VG Name                vg_root
      LV UUID                Jo5FLx-DtJ9-R4jx-4fGh-LNgU-oW51-gudmW1
      LV Write Access        read/write
      LV Creation host, time Otus, 2026-02-25 14:38:03 +0300
      LV Status              available
      # open                 1
      LV Size                <8.00 GiB
      Current LE             2047
      Segments               1
      Allocation             inherit
      Read ahead sectors     auto
      - currently set to     256
      Block device           254:2

    root@Otus:~# umount /data
    root@Otus:~# mount /dev/vg_root/lv_root /mnt

    root@Otus:~# lsblk
    NAME                MAJ:MIN RM  SIZE RO TYPE MOUNTPOINTS
    sda                   8:0    0    1G  0 disk
    sdb                   8:16   0   10G  0 disk
    `-vg_root-lv_root   254:2    0    8G  0 lvm  /mnt
    sdc                   8:32   0    2G  0 disk
    sdd                   8:48   0    1G  0 disk
    sde                   8:64   0   30G  0 disk
    |-sde1                8:65   0  487M  0 part /boot
    |-sde2                8:66   0    1K  0 part
    `-sde5                8:69   0 29.5G  0 part
      |-Otus--vg-root   254:0    0 28.6G  0 lvm  /
      `-Otus--vg-swap_1 254:1    0  976M  0 lvm  [SWAP]
    sdf                   8:80   0    1G  0 disk


Переносим /.

    root@Otus:~# rsync -avxHAX --progress / /mnt/
    rsync error: error in file IO (code 11) at receiver.c(381) [receiver=3.2.7]

    root@Otus:~# df -Th /
    Filesystem                Type  Size  Used Avail Use% Mounted on
    /dev/mapper/Otus--vg-root ext4   28G  8.3G   19G  31% /
    root@Otus:~# df -Th /mnt/
    Filesystem                  Type  Size  Used Avail Use% Mounted on
    /dev/mapper/vg_root-lv_root ext4  7.8G  7.8G     0 100% /mnt

    root@Otus:~# lvextend -l+80%FREE /dev/vg_root/lv_root
      Size of logical volume vg_root/lv_root changed from <8.00 GiB (2047 extents) to <11.20 GiB (2866 extents).
      Logical volume vg_root/lv_root successfully resized.

    root@Otus:~# resize2fs /dev/vg_root/lv_root
    resize2fs 1.47.0 (5-Feb-2023)
    Filesystem at /dev/vg_root/lv_root is mounted on /mnt; on-line resizing required
    old_desc_blocks = 1, new_desc_blocks = 2
    The filesystem on /dev/vg_root/lv_root is now 2934784 (4k) blocks long.
    root@Otus:~# df -Th /mnt/
    Filesystem                  Type  Size  Used Avail Use% Mounted on
    /dev/mapper/vg_root-lv_root ext4   11G  7.8G  2.7G  75% /mnt

    root@Otus:~# rsync -avxHAX --progress / /mnt/
    sending incremental file list
    sent 497.197.952 bytes  received 26.332 bytes  4.160.872,67 bytes/sec
    total size is 8.567.813.646  speedup is 17,23

Обновляем загрузчик.

    root@Otus:~# for i in /proc/ /sys/ /dev/ /run/ /boot/; do mount --bind $i /mnt/$i; done
    root@Otus:~# chroot /mnt/
    root@Otus:/# grub-mkconfig -o /boot/grub/grub.cfg
    Generating grub configuration file ...
    Found background image: /usr/share/images/desktop-base/desktop-grub.png
    Found linux image: /boot/vmlinuz-6.13.2-061302-generic
    Found initrd image: /boot/initrd.img-6.13.2-061302-generic
    Found linux image: /boot/vmlinuz-6.1.130
    Found initrd image: /boot/initrd.img-6.1.130
    Warning: os-prober will not be executed to detect other bootable partitions.
    Systems on them will not be added to the GRUB boot configuration.
    Check GRUB_DISABLE_OS_PROBER documentation entry.
    done
    root@Otus:/# update-initramfs -u
    perl: warning: Setting locale failed.
    perl: warning: Setting locale failed.
    perl: warning: Please check that your locale settings:
    perl: warning: Please check that your locale settings:
        LANGUAGE = (unset),
        LANGUAGE = (unset),
        LC_ALL = (unset),
        LC_ALL = (unset),
        LC_CTYPE = "UTF-8",
        LC_CTYPE = "UTF-8",
        LANG = "ru_RU.UTF-8"
        LANG = "ru_RU.UTF-8"
        are supported and installed on your system.
        are supported and installed on your system.
    perl: warning: Falling back to a fallback locale ("ru_RU.UTF-8").
    perl: warning: Falling back to a fallback locale ("ru_RU.UTF-8").
    update-initramfs: Generating /boot/initrd.img-6.13.2-061302-generic
    W: mkconf: MD subsystem is not loaded, thus I cannot scan for arrays.
    W: mdadm: failed to auto-generate temporary mdadm.conf file.

После перезагрузки.

    root@Otus:~# lsblk
    NAME                MAJ:MIN RM  SIZE RO TYPE MOUNTPOINTS
    sda                   8:0    0   10G  0 disk
    `-vg_root-lv_root   254:0    0 11.2G  0 lvm  /
    sdb                   8:16   0    2G  0 disk
    `-vg_root-lv_root   254:0    0 11.2G  0 lvm  /
    sdc                   8:32   0    1G  0 disk
    sdd                   8:48   0    1G  0 disk
    sde                   8:64   0    1G  0 disk
    sdf                   8:80   0   30G  0 disk
    |-sdf1                8:81   0  487M  0 part /boot
    |-sdf2                8:82   0    1K  0 part
    `-sdf5                8:85   0 29.5G  0 part
      |-Otus--vg-root   254:1    0 28.6G  0 lvm
      `-Otus--vg-swap_1 254:2    0  976M  0 lvm  [SWAP]


    root@Otus:~# lvremove /dev/Otus-vg/root
    Do you really want to remove active logical volume Otus-vg/root? [y/n]: y
      Logical volume "root" successfully removed.

    root@Otus:~# lvcreate -n Otus-vg/root -L 10G /dev/Otus-vg
    WARNING: ext4 signature detected on /dev/Otus-vg/root at offset 1080. Wipe it? [y/n]: y
      Wiping ext4 signature on /dev/Otus-vg/root.
      Logical volume "root" created.

    root@Otus:~# mkfs.ext4 /dev/Otus-vg/root
    mke2fs 1.47.0 (5-Feb-2023)
    Discarding device blocks: done
    Creating filesystem with 2621440 4k blocks and 655360 inodes
    Filesystem UUID: 34b88680-52cb-4470-8808-d4a8fe45b12f
    Superblock backups stored on blocks:
        32768, 98304, 163840, 229376, 294912, 819200, 884736, 1605632

    Allocating group tables: done
    Writing inode tables: done
    Creating journal (16384 blocks): done
    Writing superblocks and filesystem accounting information: done

    root@Otus:~# mount /dev/Otus-vg/root /mnt/
    root@Otus:~# rsync -avxHAX -q / /mnt/
    file has vanished: "/var/log/journal/188d0db0d6ef4f04864141635f932c10/system@8fa3b28eeeb94cad8a8707421f617891-00000000000331f8-00064366d3ca4115.journal"
    rsync warning: some files vanished before they could be transferred (code 24) at main.c(1338) [sender=3.2.7]

    root@Otus:~# for i in /proc/ /sys/ /dev/ /run/ /boot/; do mount --bind $i /mnt/$i; done
    root@Otus:~# chroot /mnt/
    root@Otus:/# grub-mkconfig -o /boot/grub/grub.cfg
    Generating grub configuration file ...
    Found background image: /usr/share/images/desktop-base/desktop-grub.png
    Found linux image: /boot/vmlinuz-6.13.2-061302-generic
    Found initrd image: /boot/initrd.img-6.13.2-061302-generic
    Found linux image: /boot/vmlinuz-6.1.130
    Found initrd image: /boot/initrd.img-6.1.130
    Warning: os-prober will not be executed to detect other bootable partitions.
    Systems on them will not be added to the GRUB boot configuration.
    Check GRUB_DISABLE_OS_PROBER documentation entry.
    done

    root@Otus:/# update-initramfs -u
    perl: warning: Setting locale failed.
    perl: warning: Setting locale failed.
    perl: warning: Please check that your locale settings:
    perl: warning: Please check that your locale settings:
        LANGUAGE = (unset),
        LANGUAGE = (unset),
        LC_ALL = (unset),
        LC_ALL = (unset),
        LC_CTYPE = "UTF-8",
        LANG = "ru_RU.UTF-8"
        LC_CTYPE = "UTF-8",
        are supported and installed on your system.
        LANG = "ru_RU.UTF-8"
        are supported and installed on your system.
    perl: warning: Falling back to a fallback locale ("ru_RU.UTF-8").
    perl: warning: Falling back to a fallback locale ("ru_RU.UTF-8").
    update-initramfs: Generating /boot/initrd.img-6.13.2-061302-generic
    W: mkconf: MD subsystem is not loaded, thus I cannot scan for arrays.
    W: mdadm: failed to auto-generate temporary mdadm.conf file.

# Зеркало под /var

    root@Otus:/# pvcreate /dev/sdc /dev/sdd
      Physical volume "/dev/sdc" successfully created.
      Physical volume "/dev/sdd" successfully created.
    root@Otus:/# vgcreate vg_var /dev/sdc /dev/sdd
      Volume group "vg_var" successfully created
    root@Otus:/# lvcreate -L 950M -m1 -n lv_var vg_var
      Rounding up size to full physical extent 952.00 MiB
      Logical volume "lv_var" created.
    root@Otus:/# mkfs.ext4 /dev/vg_var/lv_var
    mke2fs 1.47.0 (5-Feb-2023)
    Discarding device blocks: done
    Creating filesystem with 243712 4k blocks and 60928 inodes
    Filesystem UUID: 492d0b9a-f857-48f2-aa33-75506be56b16
    Superblock backups stored on blocks:
        32768, 98304, 163840, 229376

    Allocating group tables: done
    Writing inode tables: done
    Creating journal (4096 blocks): done
    Writing superblocks and filesystem accounting information: done

    root@Otus:/# mount /dev/vg_var/lv_var /mnt
    root@Otus:/# cp -aR /var/* /mnt/
    root@Otus:/# umount /mnt
    root@Otus:/# mount /dev/vg_var/lv_var /var
    root@Otus:/# echo "`blkid | grep var: | awk '{print $2}'` /var ext4 defaults 0 0" >> /etc/fstab

После перезагрузки

    root@Otus:/# lvremove /dev/vg_root/lv_root
    Do you really want to remove and DISCARD active logical volume vg_root/lv_root? [y/n]: y
      Logical volume "lv_root" successfully removed.

    root@Otus:/# vgremove /dev/vg_root
      Volume group "vg_root" successfully removed

    root@Otus:/# pvremove /dev/sdb
      Labels on physical volume "/dev/sdb" successfully wiped.


# Выделить том под /home.

    root@Otus:/# lvcreate -n LogVol_Home -L 2G /dev/Otus-vg
      Logical volume "LogVol_Home" created.

    root@Otus:/# mkfs.ext4 /dev/Otus-vg/LogVol_Home
    root@Otus:/# mount /dev/Otus-vg/LogVol_Home /mnt/
    root@Otus:/# cp -aR /home/* /mnt/
    root@Otus:/# rm -rf /home/*
    root@Otus:/# umount /mnt
    root@Otus:/# mount /dev/Otus-vg/LogVol_Home /home/
    root@Otus:/# echo "`blkid | grep Home | awk '{print $2}'` /home xfs defaults 0 0" >> /etc/fstab

# Снапшоты.

    root@Otus:/# touch /home/file{1..20}
    root@Otus:/# lvcreate -L 100MB -s -n home_snap /dev/Otus-vg/LogVol_Home
    root@Otus:/# rm -f /home/file{11..20}

    root@Otus:/# umount /home
    root@Otus:/# lvconvert --merge /dev/Otus-vg/home_snap
      Merging of volume Otus-vg/home_snap started.
      Otus-vg/LogVol_Home: Merged: 100.00%
    root@Otus:/# mount /dev/mapper/Otus--vg-LogVol_Home /home
    root@Otus:/# ls -al /home
    total 28
    drwxr-xr-x  4 root    root     4096 Dec 23 11:50 .
    drwxr-xr-x 24 root    root     4096 Dec 23 09:49 ..
    -rw-r--r--  1 root    root        0 Dec 23 11:50 file1
    -rw-r--r--  1 root    root        0 Dec 23 11:50 file10
    -rw-r--r--  1 root    root        0 Dec 23 11:50 file11
    …
