<!DOCTYPE html>
<head>
  <style media="screen">
  body {
    width: 100%;
  }

  .list {
    padding-top: 10px;
  }

  .users-img {
    width: 100%;
    height: 200px;
    margin: 20px;
  }

  .users-text {
    font-size: 15px;
    display:block;
    vertical-align: middle;
  }

  .users-address {
    float: left;
    height: 20px;
  }

  .users_reserve {
    height: 30px;
    margin-top: 20px;
    margin-bottom: 20px;
  }

  .user-detail {
    height: auto;
  }

  h2 {
    font-size: 20px;
  }

  h1 {
    font-size: 15px;
  }

  .users-card {
    /* Add shadows to create the "card" effect */
    box-shadow: 0 4px 8px 0 rgba(0,0,0,0.2);
    transition: 0.3s;
    padding: 10px;
    margin: auto;
    border-radius: 10px;
    width: 90%;
  }

  /* On mouse-over, add a deeper shadow */
  .users-card:hover {
    box-shadow: 0 8px 16px 0 rgba(0,0,0,0.2);
  }
  </style>
</head>

<body>
  <div class="list" id="users-list">
    <!-- <div class="users-card">
      <img class="users-img" src="img/seoul.png">
      <h2 id="stadium-name">송이공원</h2>
      <span><img class="users-address" src="img/custominfo_location.png"><p class="users-text" id="detail-address">서울시 송파구 송파2동</p></span>
      <h1 id="date">2017년 6월 14일</h1>
      <h1 id="hour">17시 00분부터 2시간</h1>
    </div> -->
  </div>

    <script type="text/javascript">
    var reserveInfo = null;
    var isReserve = localStorage.getItem('reserveNum');

    console.log(isReserve);

    if(isReserve) { // 사용자가 예약한 내역이 있을 경우
      readReserve();
    }
    else { // 사용자가 예약한 내역이 없을 경우
    }

    function readReserve() {
      console.log("readReserve");
      reserveInfo = $.ajax({
        type: 'POST',
        url: 'http://localhost/database/readUser.php',
        data: { reserve : isReserve },
        crossDomain: false,
        dataType: 'json',
        async: false,
        error : function(request,status,error) {
          alert("code:" + request.status + "\n" + "message:" + request.responseText + "\n" + "error:" + error);
          //history.go(0);
        }
      }).responseText;

      reserveInfo = JSON.parse(reserveInfo);
      console.log(reserveInfo);
      insertInfo();
    } // readReserve

    function insertInfo() {
      console.log("insertInfo");

      var div = document.createElement('div');
      div.className = "users-card";

      var img = document.createElement('img');
      img.className = "users-img";
      img.src="img/seoul.png";

      var h2 = document.createElement('h2');
      h2.id = "stadium-name";
      h2.innerHTML = reserveInfo[0];

      div.appendChild(img);
      div.appendChild(h2);

      var img2 = document.createElement('img');
      img2.className = "users-address";
      img2.src = "img/custominfo_location.png";

      var p = document.createElement('p');
      p.className = "users-text";
      p.id = "detail-address";
      p.innerHTML = reserveInfo[1] + ' ' + reserveInfo[2] + ' ' + reserveInfo[3];

      var span = document.createElement('span');
      span.appendChild(img2);
      span.appendChild(p);

      div.appendChild(span);

      var h1 = document.createElement('h1');
      h1.id = "date";
      h1.innerHTML = reserveInfo[4];

      div.appendChild(h1);

      var h1_2 = document.createElement('h1');
      h1_2.id = "hour";
      h1_2.innerHTML = reserveInfo[5] + '부터 ' + reserveInfo[6] + '시간';

      div.appendChild(h1_2);

      var list = document.getElementById('users-list');
      list.appendChild(div);
    }
    </script>
</body>
