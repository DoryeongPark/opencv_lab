<!DOCTYPE html>
<head>
  <meta charset="utf-8">
  <meta http-equiv="Content-Type" content="text/html;charset=utf-8">
  <meta name="viewport" content="initial-scale=1, maximum-scale=1, user-scalable=no, width=device-width">
  <script src="js/jquery-3.0.0.min.js"></script>
  <title>RSMS</title>

  <style>
  #back-button {
    width: 20px;
    height: 20px;
    margin-left: 10px;
    margin-bottom: 5px;
    float: left;
  }

  #stadium-list {
    padding-bottom: 54px;
  }
  </style>
</head>

<body>
  <div id="stadium-list">
    <!-- <div class="card" id="4">
      <img src="img/seoul.png" alt="Avatar" style="width:100%; height: 200px;">
        <div class="container">
          <h4><b>시설이름 1</b></h4>
          <p>시설주소</p>
        </div>
    </div>
    <div class="card" id="5">
      <img src="img/seoul.png" alt="Avatar" style="width:100%;  height: 200px;">
        <div class="container">
          <h4><b>시설이름 2</b></h4>
          <p>시설주소</p>
        </div>
    </div> -->
  </div> <!-- stadium-list -->

  <script type="text/javascript">
  var result;
  var storage;
  var length;

  var previous = localStorage.getItem('previous');

  readData();

  if(length != 0)
  {
    var i;

    for(i=0; i<length; i++)
    {
      addStadium(i);
    }
  }
  else {
    alert('데이터 없음');
  }

  function readData() {
    // 사용자가 선택한 지역 혹은 카테고리의 시설을 불러오기 위해서 localStorage에서 정보를 가져옴
    if(previous == 'community') {
      /* 이전 페이지가 Community이면 */
      var storage = localStorage.getItem('city');
      console.log("storage : " + storage);

      // 데이터베이스로부터 선택한 지역의 시설을 모두 불러옴
      var jsonData = $.ajax({
        type: "POST",
        url:"http://localhost/database/readStadium.php",
        data: { city : storage },
        crossDomain: true,
        cache: false,
        dataType: 'json',
        async: false,
        error : function(request,status,error) {
          alert("code:" + request.status + "\n" + "message:" + request.responseText + "\n" + "error:" + error);
          //history.go(0);
        }
      }).responseText;

      // 불러온 데이터를 파싱하여 result 변수에 저장
      result = JSON.parse(jsonData);
      length = result.length;
    }
    else {
      /* 이전 페이지가 Category이면 */
      storage = localStorage.getItem('category');
      console.log("storage : " + storage);

      // 데이터베이스로부터 선택한 지역의 시설을 모두 불러옴
      var jsonData = $.ajax({
        type: "POST",
        url:"http://localhost/database/readCategory.php",
        data: { category : storage },
        crossDomain: true,
        cache: false,
        dataType: 'json',
        async: false,
        error : function(request,status,error) {
          alert("code:" + request.status + "\n" + "message:" + request.responseText + "\n" + "error:" + error);
          //history.go(0);
        }
      }).responseText;

      // 불러온 데이터를 파싱하여 result 변수에 저장
      result = JSON.parse(jsonData);
      length = result.length;
    }
  } // readData

  function addBackDetail() {
    /* back button 추가용 */
    var back = document.createElement('img');
    back.id = 'back-button-detail';
    back.src = 'img/back_button.png';

    var header = document.getElementById('Header')
    header.insertBefore(back, header.childNodes[0]);
  } // addBackDetail

  function addBack() {
    /* back button 추가용 */
    var back = document.createElement('img');
    back.id = 'back-button';
    back.src = 'img/back_button.png';

    var header = document.getElementById('Header')
    header.insertBefore(back, header.childNodes[0]);
  } // addBack

  function addStadium(id) {
    var div = document.createElement('div');
    div.className = 'card';
    div.id = result[id][0];

    var img = document.createElement('img');
    img.src = 'img/seoul.png';
    img.style.width = '100%';
    img.style.height = '200px';

    var contain = document.createElement('div');
    contain.className = 'container';

    var name = document.createElement('h4');
    name.textContent = result[id][1];

    var address = document.createElement('p');
    address.innerHTML = result[id][2] + ' ' + result[id][3] + ' ' + result[id][4];

    contain.appendChild(name);
    contain.appendChild(address);

    div.appendChild(img);
    div.appendChild(contain);

    var list = document.getElementById('stadium-list');
    list.appendChild(div);
  } // addStadium

  function removeBack() {
    // back-button을 삭제하는 함수
    var header = document.getElementById('Header');
    var back = document.getElementById('back-button');

    header.removeChild(back);
  } // removeBack

  $('.card').on('click', function(e) {
    /* 사용자가 특정 시설을 눌렀을 경우에 해당 시설의 세부 정보 메뉴로 이동 */
    var stadium = e.currentTarget.id;
    console.log(stadium);
    localStorage.setItem('stadium', stadium);

    $.ajax({
      type : "GET",
      url : "detail.php",
      dataType : "html",
      error : function() {
        history.go(0);
      },
      success : function(data) {
        removeBack();
        addBackDetail();
        $('#Content').html(data);
      }
    });
  }); // card class click event

  $('#back-button').on('click', function() {
    $('#back-button').unbind('click');
    /* 사용자가 back-button을 눌렀을 시, 이전 화면으로 이동하는 이벤트 */
    alert('back-botton click');

    if(previous == 'community') // 이전 화면이 지역별 메뉴
    {
      $.ajax({
        type : "GET",
        url : "community.php",
        dataType : "html",
        error : function() {
          history.go(0);
        },
        success : function(data) {
          removeBack();
          $('#Content').html(data);
        }
      });
    }
    else // 이전 화면이 카테고리별 메뉴
    {
      $.ajax({
        type : "GET",
        url : "category.php",
        dataType : "html",
        error : function() {
          history.go(0);
        },
        success : function(data) {
          removeBack();
          $('#Content').html(data);
        }
      });
    }
  }); // back-button click event
  </script>
</body>
