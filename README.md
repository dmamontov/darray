\SLOBEL\DArray
==============

PHP extension. Simple array Traversable and ArrayAccess

## PHP

```bash
git clone http://git.php.net/repository/php-src.git
cd php-src
git checkout PHP-5.6
sudo apt-get install build-essential autoconf automake libtool
sudo apt-get install libxml2-dev
./buildconf
./configure --disable-all --enable-debug --prefix=$HOME/dev/bin/php
make && make install
```

## Module

```bash
git clone https://github.com/dmamontov/darray
cd darray
~/dev/bin/php/bin/phpize
./configure --with-php-config=$HOME/dev/bin/php/bin/php-config
make && make install
```

## Run the script using the module

```bash
~/dev/bin/php/bin/php -dextension=slobel.so slobel.php
```