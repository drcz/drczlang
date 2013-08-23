<?php
function SE_car($e) { if(count($e)>0) return $e[0]; else return null; }
function SE_cdr($e) { array_shift($e); return $e; }
function SE_cons($h,$t) {
    array_unshift($t,$h);
    return $t;
}
?>
