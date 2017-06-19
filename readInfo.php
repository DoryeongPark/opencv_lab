<?php
include "db.php";
mysqli_set_charset($con, 'utf8');

/* 사용자가 선택한 도시에 속하는 모든 체육시설의 정보 불러옴 */
if(isset($_POST['stadium']))
{
  $stadium = $_POST['stadium'];
  $sql = "SELECT * FROM STADIUM WHERE id = $stadium";

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
