<?php
 header("Access-Control-Allow-Origin: *");
 $con = mysqli_connect("localhost","root","password","rsms") or die ("could not connect database");

 mysqli_query($con, "set session character_set_connection=utf8;");
 mysqli_query($con, "set session character_set_results=utf8;");
 mysqli_query($con, "set session character_set_client=utf8;");
?>
