<?php
include "db.php";
mysqli_set_charset($con, 'utf8');

/* 사용자가 선택한 도시에 속하는 모든 체육시설의 정보 불러옴 */
if(isset($_POST['lat']) && isset($_POST['lng']))
{
  $latitude = $_POST['lat'];
  $longitude = $_POST['lng'];
  $sql = "SELECT *, ( 6371 * acos( cos( radians($latitude) ) * cos( radians( latitude ) ) * cos( radians( longitude ) - radians($longitude) ) + sin( radians($latitude) ) * sin( radians( latitude ) ) ) ) AS distance
          FROM rsms.stadium
          HAVING distance < 100
          ORDER BY distance
          LIMIT 0 , 5";

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
