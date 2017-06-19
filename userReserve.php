<?php
include "db.php";
mysqli_set_charset($con, 'utf8');

if(isset(isset($_POST['action'])
{
  $action = $_POST['action'];

  if($_POST['action'] == 'cancel') { // 가예약을 취소한다면
    $reserveNum = $_POST['reserveNum'];

    $sql = 'DELETE FROM RESERVE WHERE num=' + $reserveNum;
    $query = mysqli_query($con, $sql);
  }
  else { // 가예약을 한다면
    $sql = 'INSERT INTO RESERVE VALUES(' + $_POST['userId'] + ')';
  }
}

?>
