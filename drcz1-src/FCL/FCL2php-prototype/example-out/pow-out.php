<?php
require_once('fclops.php');
function proc_init($n,$m) {
init: $r=1; goto test; 
test: if($n==0) goto stop; else goto loop; 
loop: $r=($r*$m); $n=($n-1); goto test; 
stop: return  $r ; 

}
echo json_encode(proc_init(json_decode($argv[1]),json_decode($argv[2]))) . "\n";
?>
