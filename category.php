<!DOCTYPE html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="initial-scale=1, maximum-scale=1, user-scalable=no, width=device-width">
  <title>RSMS</title>

  <style>
  .category-list {
    padding: 20px;
    padding-top: 40px;
    padding-bottom: 54px;
  }

  .category-button-left {
    width: 120px;
    height: 120px;
    margin: 30px;
    margin-right: 10px !important;
    margin-bottom: 10px !important;
    text-align: center;
    background-position: center;
    background-size: cover;
  }

  .category-button-right {
    width: 120px;
    height: 120px;
    margin: 30px;
    margin-left: 10px !important;
    margin-bottom: 10px !important;
    text-align: center;
    background-position: center;
    background-size: cover;
  }
  </style>
</head>

<body>
  <div class="category-list" id="category-list">
    <div class="row">
      <div class="col-xs-6">
        <div class="category-button-left" id="간이운동장"></div>
      </div>
      <div class="col-xs-6">
        <div class="category-button-right" id="축구장"></div>
      </div>
    </div>
    <div class="row">
      <div class="col-xs-6">
        <div class="category-button-left" id="농구장"></div>
      </div>
      <div class="col-xs-6">
        <div class="category-button-right" id="야구장"></div>
      </div>
    </div>
  </div>

  <script type="text/javascript">
  // 각 지역의 배경이미지 삽입
  backgroundImage('간이운동장', 'simple');
  backgroundImage('축구장', 'soccer');
  backgroundImage('농구장', 'basketball');
  backgroundImage('야구장', 'baseball');

  function backgroundImage(itemName, imgName) {
    /* 특정 지역의 배경이미지를 삽입하는 함수 */
    var imgUrl = 'url(img/' + imgName + '.png)';
    document.getElementById(itemName).style.backgroundImage =  imgUrl;
    document.getElementById(itemName).style.backgroundRepeat = 'no-repeat';
  }

  var name = ['간이운동장', '축구장', '농구장', '야구장'];
  var id = ['simple', 'soccer', 'basketball', 'baseball'];
  console.log(name);
  console.log(id);

  //addCategory();

  // function addCategory() {
  //   var category = document.getElementById('category-list');
  //   var i;
  //
  //   for(i=0; i<3; i+2)
  //   {
  //     var row = document.createElement('div');
  //     row.class = 'row';
  //
  //     var col_left = document.createElement('div');
  //     col_left.class = 'col-xs-6';
  //
  //     var imgUrl_left = 'url(img/' + id[i] + '.png)';
  //     var button_left = document.createElement('div');
  //     button_left.class = "category-button-left";
  //     button_left.id = name[i];
  //     button_left.style.backgroundImage = imgUrl_left;
  //     button_left.style.backgroundRepeat = 'no-repeat';
  //
  //     col_left.appendChild(button_left);
  //
  //     var col_right = document.createElement('div');
  //     col_right.class = 'col-xs-6';
  //
  //     var imgUrl_right = 'url(img/' + id[i+1] + '.png)';
  //     var button_right = document.createElement('div');
  //     button_right.class = "category-button-right";
  //     button_right.id = name[i+1];
  //     button_right.style.backgroundImage = imgUrl_right;
  //     button_right.style.backgroundRepeat = 'no-repeat';
  //
  //     col_right.appendChild(button_right);
  //
  //     row.appendChild(col_left);
  //     row.appendChild(col_right);
  //
  //     category.appendChild(row);
  //   }
  // } // addCategory

  function addBack() {
    /* back button 추가용 */
    var back = document.createElement('img');
    back.id = 'back-button';
    back.src = 'img/back_button.png';

    var header = document.getElementById('Header')
    header.insertBefore(back, header.childNodes[0]);
  }

  /* 각 카테고리별 아이템의 클릭이벤트 등록 */
  $('.category-button-left').on('click', function(e) {
    //alert(e.target.id); // 이걸 가지고 php로...

    var category = document.getElementById('category-list');
    category.style.padding = '0px';
    category.style.paddingTop = '0px';
    category.style.paddingBottom = '54px';

    var dataString = e.target.id;
    console.log('selected Category : ' + dataString);
    localStorage.setItem('previous', 'category');
    localStorage.setItem('category', dataString);

    $.ajax({
        type : "GET",
        url : "list.php",
        dataType : "html",
        data: { category : dataString },
        error : function() {
          history.go(0);
        },
        success : function(data) {
          addBack();
          $('#Content').html(data);
        }
      });
  });

  /* 각 카테고리별 아이템의 클릭이벤트 등록 */
  $('.category-button-right').on('click', function(e) {
    //alert(e.target.id); // 이걸 가지고 php로...

    category.style.padding = '0px';
    category.style.paddingTop = '0px';
    category.style.paddingBottom = '54px';

    var dataString = e.target.id;
    localStorage.setItem('previous', 'category');
    localStorage.setItem('category', dataString);

    $.ajax({
        type : "GET",
        url : "list.php",
        dataType : "html",
        data: { category : dataString },
        error : function() {
          history.go(0);
        },
        success : function(data) {
          addBack();
          $('#Content').html(data);
        }
      });
  });
  </script>
</body>
