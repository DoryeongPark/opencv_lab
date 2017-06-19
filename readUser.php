<?php
include "db.php";
mysqli_set_charset($con, 'utf8');

/* 사용자의 예약 정보를 불러옴 */
if(isset($_POST['reserve']))
{
  $reserve = $_POST['reserve'];

  $sql = "SELECT name, city, district, neighbor, time, start, hour FROM RSMS.RESERVE_1 JOIN RSMS.STADIUM ON RESERVE_1.id = STADIUM.id WHERE num = $reserve";

  $query = mysqli_query($con, $sql);

  if($row = mysqli_fetch_row($query))
  echo json_encode($row, JSON_UNESCAPED_UNICODE);
}
else {
  echo "not";
}
?>
