# Заданиe 1. Обновление ядра системы.

## Постановка.
1. Запустить ВМ c Ubuntu.
2. Обновить ядро ОС на новейшую стабильную версию из mainline-репозитория.
3. Оформить отчет в README-файле в GitHub-репозитории.

## Выполнение.
Работаю на MacOS, использую UTM (QEMU). Для удобства решил использовать виртуальную машину, которая
была создана для прохождения курса "[Разработка ядра
Linux](https://otus.ru/lessons/linux-kernel/)". На ней уже был настроен ssh и для нее я уже выполнял
упражнене [сборка ядра с включенным дебагом](https://github.com/lompy/HW_01_build_kernel).

### Исходное состояние.
    student@Otus:~$ uname -r
    6.1.130
    student@Otus:~$ mkdir kernel && cd kernel

### Закачка пакетов.
    student@Otus:~/kernel$ wget https://kernel.ubuntu.com/mainline/v6.13.2/amd64/linux-headers-6.13.2-061302-generic_6.13.2-061302.202502081010_amd64.deb

<details>
  <summary>stdout</summary>
    --2025-10-15 14:29:21--  https://kernel.ubuntu.com/mainline/v6.13.2/amd64/linux-headers-6.13.2-061302-generic_6.13.2-061302.202502081010_amd64.deb
    Resolving kernel.ubuntu.com (kernel.ubuntu.com)... 185.125.189.74, 185.125.189.76, 185.125.189.75
    Connecting to kernel.ubuntu.com (kernel.ubuntu.com)|185.125.189.74|:443... connected.
    HTTP request sent, awaiting response... 200 OK
    Length: 3694072 (3.5M) [application/x-debian-package]
    Saving to: 'linux-headers-6.13.2-061302-generic_6.13.2-061302.202502081010_amd64.deb'

    linux-headers-6.13.2-061302-generic_6. 100%[==========================================================================>]   3.52M  4.21MB/s    in 0.8s

    2025-10-15 14:29:33 (4.21 MB/s) - 'linux-headers-6.13.2-061302-generic_6.13.2-061302.202502081010_amd64.deb' saved [3694072/3694072]
</details>

    student@Otus:~/kernel$ wget https://kernel.ubuntu.com/mainline/v6.13.2/amd64/linux-headers-6.13.2-061302_6.13.2-061302.202502081010_all.deb

<details>
  <summary>stdout</summary>
    --2025-10-15 14:29:39--  https://kernel.ubuntu.com/mainline/v6.13.2/amd64/linux-headers-6.13.2-061302_6.13.2-061302.202502081010_all.deb
    Resolving kernel.ubuntu.com (kernel.ubuntu.com)... 185.125.189.74, 185.125.189.76, 185.125.189.75
    Connecting to kernel.ubuntu.com (kernel.ubuntu.com)|185.125.189.74|:443... connected.
    HTTP request sent, awaiting response... 200 OK
    Length: 13875326 (13M) [application/x-debian-package]
    Saving to: 'linux-headers-6.13.2-061302_6.13.2-061302.202502081010_all.deb'

    linux-headers-6.13.2-061302_6.13.2-061 100%[==========================================================================>]  13.23M  4.22MB/s    in 3.1s

    2025-10-15 14:29:43 (4.22 MB/s) - 'linux-headers-6.13.2-061302_6.13.2-061302.202502081010_all.deb' saved [13875326/13875326]
</details>

    student@Otus:~/kernel$ wget https://kernel.ubuntu.com/mainline/v6.13.2/amd64/linux-image-unsigned-6.13.2-061302-generic_6.13.2-061302.202502081010_amd64.deb
<details>
  <summary>stdout</summary>
    --2025-10-15 14:29:51--  https://kernel.ubuntu.com/mainline/v6.13.2/amd64/linux-image-unsigned-6.13.2-061302-generic_6.13.2-061302.202502081010_amd64.deb
    Resolving kernel.ubuntu.com (kernel.ubuntu.com)... 185.125.189.74, 185.125.189.76, 185.125.189.75
    Connecting to kernel.ubuntu.com (kernel.ubuntu.com)|185.125.189.74|:443... connected.
    HTTP request sent, awaiting response... 200 OK
    Length: 15677632 (15M) [application/x-debian-package]
    Saving to: 'linux-image-unsigned-6.13.2-061302-generic_6.13.2-061302.202502081010_amd64.deb'

    linux-image-unsigned-6.13.2-061302-gen 100%[==========================================================================>]  14.95M  9.97MB/s    in 1.5s

    2025-10-15 14:29:54 (9.97 MB/s) - 'linux-image-unsigned-6.13.2-061302-generic_6.13.2-061302.202502081010_amd64.deb' saved [15677632/15677632]
</details>

    student@Otus:~/kernel$ wget https://kernel.ubuntu.com/mainline/v6.13.2/amd64/linux-modules-6.13.2-061302-generic_6.13.2-061302.202502081010_amd64.deb
<details>
  <summary>stdout</summary>
    --2025-10-15 14:30:00--  https://kernel.ubuntu.com/mainline/v6.13.2/amd64/linux-modules-6.13.2-061302-generic_6.13.2-061302.202502081010_amd64.deb
    Resolving kernel.ubuntu.com (kernel.ubuntu.com)... 185.125.189.74, 185.125.189.76, 185.125.189.75
    Connecting to kernel.ubuntu.com (kernel.ubuntu.com)|185.125.189.74|:443... connected.
    HTTP request sent, awaiting response... 200 OK
    Length: 184576192 (176M) [application/x-debian-package]
    Saving to: 'linux-modules-6.13.2-061302-generic_6.13.2-061302.202502081010_amd64.deb'

    linux-modules-6.13.2-061302-generic_6. 100%[==========================================================================>] 176.03M  2.93MB/s    in 60s

    2025-10-15 14:31:01 (2.94 MB/s) - 'linux-modules-6.13.2-061302-generic_6.13.2-061302.202502081010_amd64.deb' saved [184576192/184576192]
</details>

### Установка.
    student@Otus:~/kernel$ sudo dpkg -i *.deb

<details>
  <summary>stdout</summary>
    [sudo] password for student:
    Selecting previously unselected package linux-headers-6.13.2-061302.
    (Reading database ... 147064 files and directories currently installed.)
    Preparing to unpack linux-headers-6.13.2-061302_6.13.2-061302.202502081010_all.deb ...
    Unpacking linux-headers-6.13.2-061302 (6.13.2-061302.202502081010) ...
    Selecting previously unselected package linux-headers-6.13.2-061302-generic.
    Preparing to unpack linux-headers-6.13.2-061302-generic_6.13.2-061302.202502081010_amd64.deb ...
    Unpacking linux-headers-6.13.2-061302-generic (6.13.2-061302.202502081010) ...
    Selecting previously unselected package linux-image-unsigned-6.13.2-061302-generic.
    Preparing to unpack linux-image-unsigned-6.13.2-061302-generic_6.13.2-061302.202502081010_amd64.deb ...
    Unpacking linux-image-unsigned-6.13.2-061302-generic (6.13.2-061302.202502081010) ...
    Selecting previously unselected package linux-modules-6.13.2-061302-generic.
    Preparing to unpack linux-modules-6.13.2-061302-generic_6.13.2-061302.202502081010_amd64.deb ...
    Unpacking linux-modules-6.13.2-061302-generic (6.13.2-061302.202502081010) ...
    Setting up linux-headers-6.13.2-061302 (6.13.2-061302.202502081010) ...
    dpkg: dependency problems prevent configuration of linux-headers-6.13.2-061302-generic:
    linux-headers-6.13.2-061302-generic depends on libc6 (>= 2.38); however:
    Version of libc6:amd64 on system is 2.36-9+deb12u9.
    linux-headers-6.13.2-061302-generic depends on libelf1t64 (>= 0.144); however:
    Package libelf1t64 is not installed.
    linux-headers-6.13.2-061302-generic depends on libssl3t64 (>= 3.0.0); however:
    Package libssl3t64 is not installed.

    dpkg: error processing package linux-headers-6.13.2-061302-generic (--install):
    dependency problems - leaving unconfigured
    Setting up linux-modules-6.13.2-061302-generic (6.13.2-061302.202502081010) ...
    Setting up linux-image-unsigned-6.13.2-061302-generic (6.13.2-061302.202502081010) ...
    I: /vmlinuz is now a symlink to boot/vmlinuz-6.13.2-061302-generic
    I: /initrd.img is now a symlink to boot/initrd.img-6.13.2-061302-generic
    Processing triggers for linux-image-unsigned-6.13.2-061302-generic (6.13.2-061302.202502081010) ...
    /etc/kernel/postinst.d/initramfs-tools:
    update-initramfs: Generating /boot/initrd.img-6.13.2-061302-generic
    /etc/kernel/postinst.d/zz-update-grub:
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
    Errors were encountered while processing:
    linux-headers-6.13.2-061302-generic
</details>

Ошибка установки headers:
    dpkg: dependency problems prevent configuration of linux-headers-6.13.2-061302-generic:
     linux-headers-6.13.2-061302-generic depends on libc6 (>= 2.38); however:
      Version of libc6:amd64 on system is 2.36-9+deb12u9

Cписок в `/boot`

    student@Otus:~/kernel$ ls -al /boot
    total 150843
    drwxr-xr-x  4 root root     1024 Oct 15 14:42 .
    drwxr-xr-x 19 root root     4096 Oct 15 14:37 ..
    -rw-r--r--  1 root root  3895118 Mar 12  2025 System.map-6.1.130
    -rw-------  1 root root  9934398 Feb  8  2025 System.map-6.13.2-061302-generic
    -rw-r--r--  1 root root   259426 Mar 12  2025 config-6.1.130
    -rw-r--r--  1 root root   294303 Feb  8  2025 config-6.13.2-061302-generic
    drwxr-xr-x  5 root root     1024 Oct 15 14:42 grub
    -rw-r--r--  1 root root 48094351 Mar 13  2025 initrd.img-6.1.130
    -rw-r--r--  1 root root 67509475 Oct 15 14:42 initrd.img-6.13.2-061302-generic
    drwx------  2 root root    12288 Mar 12  2025 lost+found
    -rw-r--r--  1 root root  8192512 Mar 12  2025 vmlinuz-6.1.130
    -rw-------  1 root root 15647232 Feb  8  2025 vmlinuz-6.13.2-061302-generic


Поизучал вопрос с версией libc и в итоге решил сделать `sudo apt full-upgrade`, чтобы не разбираться
с проблемой вручную. И естественно эта команда сама взяла и сделала `sudo update-grub` в конце со
свежим ядром. При этом список в `/boot` она не поменяла, то есть видимо та же версия ядра
подтянулась. Сделал `sudo grub-set-default 0` и перезагрузился.

### Результат.
    student@Otus:~$ uname -r
    6.13.2-061302-generic
