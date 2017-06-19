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

/* 사용자가 선택한 체육시설의 현재로부터 7일 이내 예약 정보를 불러옴 */
if(isset($_POST['stadium']))
{
  $stadium = $_POST['stadium'];
  $startDate = date('Y-m-d');
  $endDate = date("Y-m-d",strtotime("+1 week"));

  $sql = "SELECT * FROM rsms.reserve_1 WHERE id = $stadium AND (time between '$startDate' and '$endDate') ORDER BY time";

  $data = array();
  $query = mysqli_query($con, $sql);

  while($row = mysqli_fetch_row($query)) {
    $data[] = $row;
  }
  echo json_encode($data, JSON_UNESCAPED_UNICODE);
}
else {
  echo "not";
}
?>
