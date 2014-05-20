<!DOCTYPE html>
<html>
  <head>
    <title>Configuration for Roberto</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="http://code.jquery.com/mobile/1.3.2/jquery.mobile-1.3.2.min.css" />
    <script src="http://code.jquery.com/jquery-1.9.1.min.js"></script>
    <script src="http://code.jquery.com/mobile/1.3.2/jquery.mobile-1.3.2.min.js"></script>
  </head>
  <body>
    <div data-role="page" id="main">
      <div data-role="header" class="jqm-header">
        <h1>Configuration options</h1>
      </div>

      <div data-role="content">

        <div data-role="fieldcontain">
          <label for="thingbrokerurl">Thingbroker URL:</label>
          <textarea maxlength="100" cols="10" rows="1" name="thingbrokerurl" id="thingbrokerurl"><?php echo isset($_GET['thingbrokerurl']) ? $_GET['thingbrokerurl'] : 'http://kimberly.magic.ubc.ca:8080/thingbroker'; ?></textarea>
        </div>

        <div class="ui-body ui-body-b">
          <fieldset class="ui-grid-a">
              <div class="ui-block-a"><button type="submit" data-theme="d" id="b-cancel">Cancel</button></div>
              <div class="ui-block-b"><button type="submit" data-theme="a" id="b-submit">Submit</button></div>
            </fieldset>
          </div>
        </div>
      </div>
    </div>
    <script>
      function saveOptions() {
        var options = {
          'thingbrokerurl': $("#thingbrokerurl").val(),
        }
        return options;
      }

      $().ready(function() {
        $("#b-cancel").click(function() {
          console.log("Cancel");
          document.location = "pebblejs://close";
        });

        $("#b-submit").click(function() {
          console.log("Submit");

          var location = "pebblejs://close#" + encodeURIComponent(JSON.stringify(saveOptions()));
          console.log("Warping to: " + location);
          console.log(location);
          document.location = location;
        });

      });
    </script>
  </body>
</html>
