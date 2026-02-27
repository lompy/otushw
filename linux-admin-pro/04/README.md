# Домашнее задание Стенд ZFS.

## Определение алгоритма с наилучшим сжатием.

Создадим 4 пула в режиме mirror.

    root@otus-admin-pro:~# lsblk
    NAME                      MAJ:MIN RM  SIZE RO TYPE MOUNTPOINTS
    vda                       253:0    0   10G  0 disk
    ├─vda1                    253:1    0  538M  0 part /boot/efi
    ├─vda2                    253:2    0  1.8G  0 part /boot
    └─vda3                    253:3    0  7.7G  0 part
      └─ubuntu--vg-ubuntu--lv 252:0    0  7.7G  0 lvm  /
    vdb                       253:16   0  512M  0 disk
    vdc                       253:32   0  512M  0 disk
    vdd                       253:48   0  512M  0 disk
    vde                       253:64   0  512M  0 disk
    vdf                       253:80   0  512M  0 disk
    vdg                       253:96   0  512M  0 disk
    vdh                       253:112  0  512M  0 disk
    vdi                       253:128  0  512M  0 disk

    root@otus-admin-pro:~# zpool create otus1 mirror /dev/vdb /dev/vdc
    root@otus-admin-pro:~# zpool create otus2 mirror /dev/vdd /dev/vde
    root@otus-admin-pro:~# zpool create otus3 mirror /dev/vdf /dev/vdg
    root@otus-admin-pro:~# zpool create otus4 mirror /dev/vdh /dev/vdi

    root@otus-admin-pro:~# zpool list
    NAME    SIZE  ALLOC   FREE  CKPOINT  EXPANDSZ   FRAG    CAP  DEDUP    HEALTH  ALTROOT
    otus1   480M   122K   480M        -         -     0%     0%  1.00x    ONLINE  -
    otus2   480M   108K   480M        -         -     0%     0%  1.00x    ONLINE  -
    otus3   480M   106K   480M        -         -     0%     0%  1.00x    ONLINE  -
    otus4   480M   141K   480M        -         -     0%     0%  1.00x    ONLINE  -

Проверим работу алгоритмов сжатия.

    root@otus-admin-pro:~# zfs set compression=lzjb otus1
    root@otus-admin-pro:~# zfs set compression=lz4 otus2
    root@otus-admin-pro:~# zfs set compression=gzip-9 otus3
    root@otus-admin-pro:~# zfs set compression=zle otus4

    root@otus-admin-pro:~# zfs get all | grep compression
    otus1  compression           lzjb                   local
    otus2  compression           lz4                    local
    otus3  compression           gzip-9                 local
    otus4  compression           zle                    local

    root@otus-admin-pro:~# for i in {1..4}; do wget -P /otus$i https://gutenberg.org/cache/epub/2600/pg2600.converter.log; done
<details>
  <summary>stdout</summary>

    --2026-02-26 15:01:48--  https://gutenberg.org/cache/epub/2600/pg2600.converter.log
    Resolving gutenberg.org (gutenberg.org)... 152.19.134.47, 2610:28:3090:3000:0:bad:cafe:47
    Connecting to gutenberg.org (gutenberg.org)|152.19.134.47|:443... connected.
    HTTP request sent, awaiting response... 200 OK
    Length: 41212364 (39M) [text/plain]
    Saving to: ‘/otus1/pg2600.converter.log’

    pg2600.converter.log                99%[=============================================================> ]  39.18M  55.4KB/s    in 2m 27s

    2026-02-26 15:04:17 (273 KB/s) - Connection closed at byte 41087415. Retrying.

    --2026-02-26 15:04:18--  (try: 2)  https://gutenberg.org/cache/epub/2600/pg2600.converter.log
    Reusing existing connection to gutenberg.org:443.
    HTTP request sent, awaiting response... No data received.
    Retrying.

    --2026-02-26 15:04:20--  (try: 3)  https://gutenberg.org/cache/epub/2600/pg2600.converter.log
    Connecting to gutenberg.org (gutenberg.org)|152.19.134.47|:443... connected.
    HTTP request sent, awaiting response... 206 Partial Content
    Length: 41212364 (39M), 124949 (122K) remaining [text/plain]
    Saving to: ‘/otus1/pg2600.converter.log’

    pg2600.converter.log               100%[++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++>]  39.30M   243KB/s    in 0.5s

    2026-02-26 15:04:22 (243 KB/s) - ‘/otus1/pg2600.converter.log’ saved [41212364/41212364]

    --2026-02-26 15:04:22--  https://gutenberg.org/cache/epub/2600/pg2600.converter.log
    Resolving gutenberg.org (gutenberg.org)... 152.19.134.47, 2610:28:3090:3000:0:bad:cafe:47
    Connecting to gutenberg.org (gutenberg.org)|152.19.134.47|:443... connected.
    HTTP request sent, awaiting response... 200 OK
    Length: 41212364 (39M) [text/plain]
    Saving to: ‘/otus2/pg2600.converter.log’

    pg2600.converter.log               100%[==============================================================>]  39.30M  1.44MB/s    in 48s

    2026-02-26 15:05:11 (846 KB/s) - ‘/otus2/pg2600.converter.log’ saved [41212364/41212364]

    --2026-02-26 15:05:11--  https://gutenberg.org/cache/epub/2600/pg2600.converter.log
    Resolving gutenberg.org (gutenberg.org)... 152.19.134.47, 2610:28:3090:3000:0:bad:cafe:47
    Connecting to gutenberg.org (gutenberg.org)|152.19.134.47|:443... connected.
    HTTP request sent, awaiting response... 200 OK
    Length: 41212364 (39M) [text/plain]
    Saving to: ‘/otus3/pg2600.converter.log’

    pg2600.converter.log               100%[==============================================================>]  39.30M  2.40MB/s    in 16s

    2026-02-26 15:05:28 (2.47 MB/s) - ‘/otus3/pg2600.converter.log’ saved [41212364/41212364]

    --2026-02-26 15:05:28--  https://gutenberg.org/cache/epub/2600/pg2600.converter.log
    Resolving gutenberg.org (gutenberg.org)... 152.19.134.47, 2610:28:3090:3000:0:bad:cafe:47
    Connecting to gutenberg.org (gutenberg.org)|152.19.134.47|:443... connected.
    HTTP request sent, awaiting response... 200 OK
    Length: 41212364 (39M) [text/plain]
    Saving to: ‘/otus4/pg2600.converter.log’

    pg2600.converter.log                80%[=================================================>             ]  31.62M   434KB/s    in 27s

    2026-02-26 15:05:57 (1.17 MB/s) - Connection closed at byte 33155895. Retrying.

    --2026-02-26 15:05:58--  (try: 2)  https://gutenberg.org/cache/epub/2600/pg2600.converter.log
    Connecting to gutenberg.org (gutenberg.org)|152.19.134.47|:443... connected.
    HTTP request sent, awaiting response... 206 Partial Content
    Length: 41212364 (39M), 8056469 (7.7M) remaining [text/plain]
    Saving to: ‘/otus4/pg2600.converter.log’

    pg2600.converter.log               100%[++++++++++++++++++++++++++++++++++++++++++++++++++============>]  39.30M   473KB/s    in 11s

    2026-02-26 15:06:10 (714 KB/s) - ‘/otus4/pg2600.converter.log’ saved [41212364/41212364]
</details>

    root@otus-admin-pro:~# zfs list
    NAME    USED  AVAIL  REFER  MOUNTPOINT
    otus1  21.8M   330M  21.6M  /otus1
    otus2  17.7M   334M  17.6M  /otus2
    otus3  10.9M   341M  10.7M  /otus3
    otus4  39.5M   313M  39.4M  /otus4

    root@otus-admin-pro:~# zfs get all | grep compressratio | grep -v ref
    otus1  compressratio         1.82x                  -
    otus2  compressratio         2.23x                  -
    otus3  compressratio         3.66x                  -
    otus4  compressratio         1.00x                  -

Получили степень сжатия

    lzjb   1.82
    lz4    2.23
    gzip-9 3.66
    zle    1.00

## Определение настроек пула.

Качаем.

    root@otus-admin-pro:~# wget -O archive.tar.gz --no-check-certificate 'https://drive.usercontent.google.com/download?id=1MvrcEp-WgAQe57aDEzxSRalPAwbNN1Bb&export=download'
    --2026-02-27 03:11:17--  https://drive.usercontent.google.com/download?id=1MvrcEp-WgAQe57aDEzxSRalPAwbNN1Bb&export=download
    Resolving drive.usercontent.google.com (drive.usercontent.google.com)... 172.217.17.193, 2a00:1450:400e:80e::2001
    Connecting to drive.usercontent.google.com (drive.usercontent.google.com)|172.217.17.193|:443... connected.
    HTTP request sent, awaiting response... 200 OK
    Length: 7275140 (6.9M) [application/octet-stream]
    Saving to: ‘archive.tar.gz’

    archive.tar.gz                     100%[==============================================================>]   6.94M  4.51MB/s    in 1.5s

    2026-02-27 03:11:25 (4.51 MB/s) - ‘archive.tar.gz’ saved [7275140/7275140]

    root@otus-admin-pro:~# tar -xzvf archive.tar.gz
    zpoolexport/
    zpoolexport/filea
    zpoolexport/fileb

Импортируем.

    root@otus-admin-pro:~# zpool import -d zpoolexport/
       pool: otus
         id: 6554193320433390805
      state: ONLINE
    status: Some supported features are not enabled on the pool.
        (Note that they may be intentionally disabled if the
        'compatibility' property is set.)
     action: The pool can be imported using its name or numeric identifier, though
        some features will not be available without an explicit 'zpool upgrade'.
     config:

        otus                         ONLINE
          mirror-0                   ONLINE
            /root/zpoolexport/filea  ONLINE
            /root/zpoolexport/fileb  ONLINE
    root@otus-admin-pro:~# zpool import -d zpoolexport/ otus

    root@otus-admin-pro:~# zpool status
<details>
  <summary>stdout</summary>

      pool: otus
     state: ONLINE
    status: Some supported and requested features are not enabled on the pool.
        The pool can still be used, but some features are unavailable.
    action: Enable all features using 'zpool upgrade'. Once this is done,
        the pool may no longer be accessible by software that does not support
        the features. See zpool-features(7) for details.
    config:

        NAME                         STATE     READ WRITE CKSUM
        otus                         ONLINE       0     0     0
          mirror-0                   ONLINE       0     0     0
            /root/zpoolexport/filea  ONLINE       0     0     0
            /root/zpoolexport/fileb  ONLINE       0     0     0

    errors: No known data errors

      pool: otus1
     state: ONLINE
    config:

        NAME        STATE     READ WRITE CKSUM
        otus1       ONLINE       0     0     0
          mirror-0  ONLINE       0     0     0
            vdb     ONLINE       0     0     0
            vdc     ONLINE       0     0     0

    errors: No known data errors

      pool: otus2
     state: ONLINE
    config:

        NAME        STATE     READ WRITE CKSUM
        otus2       ONLINE       0     0     0
          mirror-0  ONLINE       0     0     0
            vdd     ONLINE       0     0     0
            vde     ONLINE       0     0     0

    errors: No known data errors

      pool: otus3
     state: ONLINE
    config:

        NAME        STATE     READ WRITE CKSUM
        otus3       ONLINE       0     0     0
          mirror-0  ONLINE       0     0     0
            vdf     ONLINE       0     0     0
            vdg     ONLINE       0     0     0

    errors: No known data errors

      pool: otus4
     state: ONLINE
    config:

        NAME        STATE     READ WRITE CKSUM
        otus4       ONLINE       0     0     0
          mirror-0  ONLINE       0     0     0
            vdh     ONLINE       0     0     0
            vdi     ONLINE       0     0     0

    errors: No known data errors
</details>

    root@otus-admin-pro:~# zfs get all otus
    NAME  PROPERTY              VALUE                  SOURCE
    otus  type                  filesystem             -
    otus  creation              Fri May 15  4:00 2020  -
    otus  used                  2.04M                  -
    otus  available             350M                   -
    otus  referenced            24K                    -
    otus  compressratio         1.00x                  -
    otus  mounted               yes                    -
    otus  quota                 none                   default
    otus  reservation           none                   default
    otus  recordsize            128K                   local
    otus  mountpoint            /otus                  default
    otus  sharenfs              off                    default
    otus  checksum              sha256                 local
    otus  compression           zle                    local
    otus  atime                 on                     default
    otus  devices               on                     default
    otus  exec                  on                     default
    otus  setuid                on                     default
    otus  readonly              off                    default
    otus  zoned                 off                    default
    otus  snapdir               hidden                 default
    otus  aclmode               discard                default
    otus  aclinherit            restricted             default
    otus  createtxg             1                      -
    otus  canmount              on                     default
    otus  xattr                 on                     default
    otus  copies                1                      default
    otus  version               5                      -
    otus  utf8only              off                    -
    otus  normalization         none                   -
    otus  casesensitivity       sensitive              -
    otus  vscan                 off                    default
    otus  nbmand                off                    default
    otus  sharesmb              off                    default
    otus  refquota              none                   default
    otus  refreservation        none                   default
    otus  guid                  14592242904030363272   -
    otus  primarycache          all                    default
    otus  secondarycache        all                    default
    otus  usedbysnapshots       0B                     -
    otus  usedbydataset         24K                    -
    otus  usedbychildren        2.01M                  -
    otus  usedbyrefreservation  0B                     -
    otus  logbias               latency                default
    otus  objsetid              54                     -
    otus  dedup                 off                    default
    otus  mlslabel              none                   default
    otus  sync                  standard               default
    otus  dnodesize             legacy                 default
    otus  refcompressratio      1.00x                  -
    otus  written               24K                    -
    otus  logicalused           1020K                  -
    otus  logicalreferenced     12K                    -
    otus  volmode               default                default
    otus  filesystem_limit      none                   default
    otus  snapshot_limit        none                   default
    otus  filesystem_count      none                   default
    otus  snapshot_count        none                   default
    otus  snapdev               hidden                 default
    otus  acltype               off                    default
    otus  context               none                   default
    otus  fscontext             none                   default
    otus  defcontext            none                   default
    otus  rootcontext           none                   default
    otus  relatime              on                     default
    otus  redundant_metadata    all                    default
    otus  overlay               on                     default
    otus  encryption            off                    default
    otus  keylocation           none                   default
    otus  keyformat             none                   default
    otus  pbkdf2iters           0                      default
    otus  special_small_blocks  0                      default

    root@otus-admin-pro:~# zfs list
    NAME             USED  AVAIL  REFER  MOUNTPOINT
    otus            2.04M   350M    24K  /otus
    otus/hometask2  1.88M   350M  1.88M  /otus/hometask2
    otus1           21.8M   330M  21.6M  /otus1
    otus2           17.7M   334M  17.6M  /otus2
    otus3           10.9M   341M  10.7M  /otus3
    otus4           39.5M   313M  39.4M  /otus4

## Работа со снапшотом, поиск сообщения от преподавателя.

    root@otus-admin-pro:~# wget -O otus_task2.file --no-check-certificate https://drive.usercontent.google.com/download?id=1wgxjih8YZ-cqLqaZVa0lA3h3Y029c3oI&export=download
    [1] 3098
    root@otus-admin-pro:~# 
    Redirecting output to ‘wget-log’.
    
    [1]+  Done                    wget -O otus_task2.file --no-check-certificate https://drive.usercontent.google.com/download?id=1wgxjih8YZ-cqLqaZVa0lA3h3Y029c3oI
    root@otus-admin-pro:~# zfs receive otus/test@today < otus_task2.file
    root@otus-admin-pro:~# find /otus/test -name "secret_message"
    /otus/test/task1/file_mess/secret_message
    root@otus-admin-pro:~# cat /otus/test/task1/file_mess/secret_message
    https://otus.ru/lessons/linux-hl/


