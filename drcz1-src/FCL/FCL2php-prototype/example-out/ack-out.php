<?php
require_once('fclops.php');
function proc_ack($m,$n) {
ack: if($m==0) goto done; else goto next; 
next: if($n==0) goto ack0; else goto ack1; 
done: return  ($n+1) ; 
ack1: $n=($n-1); $n=proc_ack($m,$n); goto ack2; 
ack0: $n=1; goto ack2; 
ack2: $m=($m-1); $n=proc_ack($m,$n); return  $n ; 

}
echo json_encode(proc_ack(json_decode($argv[1]),json_decode($argv[2]))) . "\n";
?>
