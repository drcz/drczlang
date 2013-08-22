<?php
require_once('fclops.php');
function proc_init($a,$b) {
init: $res=proc_apd($a,$b); return  $res ; 

}
function proc_apd($a,$b) {
apd: if($a==array()) goto end; else goto loop; 
loop: $next=SE_car($a); $a=SE_cdr($a); $r=proc_apd($a,$b); $res=SE_cons($next,$r); return  $res ; 
end: return  $b ; 

}
echo json_encode(proc_init(json_decode($argv[1]),json_decode($argv[2]))) . "\n";
?>
