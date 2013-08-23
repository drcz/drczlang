<?php
require_once('fclops.php');
function proc_init($q,$right) {
init: $q_tail=$q; $left=array(); goto loop; 
loop: if($q_tail==array()) goto stop; else goto cont; 
cont: $instruction=SE_car($q_tail); $q_tail=SE_cdr($q_tail); $operator=SE_car(SE_cdr($instruction)); goto check_right; 
stop: return  $left ; 
check_right: if($operator=='RIGHT') goto do_right; else goto check_left; 
check_left: if($operator=='LEFT') goto do_left; else goto check_write; 
do_right: $left=SE_cons(SE_car($right),$left); $right=SE_cdr($right); goto loop; 
check_write: if($operator=='WRITE') goto do_write; else goto check_goto; 
do_left: $right=SE_cons(SE_car($left),$right); $left=SE_cdr($left); goto loop; 
check_goto: if($operator=='GOTO') goto do_goto; else goto check_if; 
do_write: $symbol=SE_car(SE_cdr(SE_cdr($instruction))); $right=SE_cons($symbol,SE_cdr($right)); goto loop; 
check_if: if($operator=='IF') goto do_if; else goto err; 
do_goto: $label=SE_car(SE_cdr(SE_cdr($instruction))); goto jump; 
jump: $q_tail=$q; goto jump_check; 
jump_check: if($label==SE_car(SE_car($q_tail))) goto loop; else goto jump_next; 
jump_next: $q_tail=SE_cdr($q_tail); goto jump_check; 
err: return  array('err',$instruction) ; 
do_if: $symbol=SE_car(SE_cdr(SE_cdr($instruction))); $label=SE_car(SE_cdr(SE_cdr(SE_cdr($instruction)))); if($symbol==SE_car($right)) goto jump; else goto loop; 

}
echo json_encode(proc_init(json_decode($argv[1]),json_decode($argv[2]))) . "\n";
?>
