<?php
include "db.php";
mysqli_set_charset($con, 'utf8');
date_default_timezone_set("Asia/Seoul");

// $sql = "insert into rsms.reserve_1 values(null, 'sample', '토요일', '2', '0', '2017-06-11', '11:00')";
// $result = mysqli_query($con, $sql);
//
// if($result)
//  $last_id = mysqli_insert_id($con);
//
//  echo $last_id;

/* 사용자가 선택한 체육시설을 예약함 */

if(isset($_POST['user']))
{
  $user = $_POST['user'];
  $day = $_POST['day'];
  $id = $_POST['id'];
  $time = $_POST['time'];
  $start = $_POST['start'];
  $hour = $_POST['hour'];

  $sql = "INSERT INTO RSMS.RESERVE_1 VALUES(null, '$user', '$day', '$hour', '$id', '$time', '$start')";

  $query = mysqli_query($con, $sql);

  $last_id = 0;
  if($query) {
    $last_id = mysqli_insert_id($con);
  }
  echo json_encode($last_id, JSON_UNESCAPED_UNICODE);
}
else {
  echo "not";
}
?>
