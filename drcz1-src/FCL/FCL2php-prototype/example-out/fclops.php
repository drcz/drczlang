<?php

function SE_car($e) { return $e[0]; }
function SE_cdr($e) { array_shift($e); return $e; }
function SE_cons($h,$t) {
    array_unshift($t,$h);
    return $t;
}


?>