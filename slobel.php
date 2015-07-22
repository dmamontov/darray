<?php
ini_set("memory_limit", "512M");

$data = range(1, 500000);

$t1 = microtime(true);
$m1 = memory_get_usage();

$jar = new \SLOBEL\DArray(500000, 0);
foreach($data as $index => &$val) {
	$jar[$index] = $val * 3;
}

echo "SLOBEL\Darray" . PHP_EOL;
echo "TIME: " . (microtime(true) - $t1) . PHP_EOL;
echo "MEMORY: " . ((memory_get_usage() - $m1)/1048576) . PHP_EOL;
gc_collect_cycles();

$t1 = microtime(true);
$m1 = memory_get_usage();

$nar = $jar->filter(function($n) {return $n % 2 == 0;});

echo "SLOBEL\Darray filter" . PHP_EOL;
echo "TIME: " . (microtime(true) - $t1) . PHP_EOL;
echo "MEMORY: " . ((memory_get_usage() - $m1)/1048576) . PHP_EOL;
gc_collect_cycles();


$t1 = microtime(true);
$m1 = memory_get_usage();

$nar = new \SLOBEL\DArray(250000, 10000);
$i = 0;
foreach($jar as $val) {
	if($val % 2 == 0) {
		$nar[$i] = $val;
		$i++;
	}
}

echo "SLOBEL\Darray filter foreach" . PHP_EOL;
echo "TIME: " . (microtime(true) - $t1) . PHP_EOL;
echo "MEMORY: " . ((memory_get_usage() - $m1)/1048576) . PHP_EOL;
unset($jar);
unset($newArr);
gc_collect_cycles();


$t1 = microtime(true);
$m1 = memory_get_usage();
$ar = [];

foreach($data as $index => &$val) {
	$ar[$index] = $val * 3;
}

echo "Array" . PHP_EOL;
echo "TIME: " . (microtime(true) - $t1) . PHP_EOL;
echo "MEMORY: " . ((memory_get_usage() - $m1)/1048576) . PHP_EOL;
gc_collect_cycles();


$t1 = microtime(true);
$m1 = memory_get_usage();
$newAr = [];

foreach($ar as $index => &$val) {
	if($val % 2 == 0) {
		$newAr[] = $val;
	}
}

echo "Array filter" . PHP_EOL;
echo "TIME: " . (microtime(true) - $t1) . PHP_EOL;
echo "MEMORY: " . ((memory_get_usage() - $m1)/1048576) . PHP_EOL;

gc_collect_cycles();
