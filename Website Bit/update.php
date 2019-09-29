<?php
exec("./md5script.sh 2>&1", $output);
echo $output[0];
?>