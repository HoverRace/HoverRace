<?
	// SPLIT THE TRACK NAME BIT AWAY
    $split = split ("rec",$command[2]);
    $Search= split (" ",urldecode($split[1]),2);
    $Search[1] = str_replace("'", "'''", $Search[1]);
    
    if ($Search[1]==FALSE)
    {
        $resultmsg = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','$addchatid[0]','IMR','You left the search blank (Usage: /rec <track name>).','".time()."')",$db);
        exit;
    } else if (strlen($Search[1])<=3)
    {
        $resultmsg = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','$addchatid[0]','IMR','Your search must be a minimum of 3 characters.','".time()."')",$db);
        exit;
    }

    $dbtrack = mysql_connect($serverip, "sirbrock_tracks", $serverpassword);
	mysql_select_db("sirbrock_tracks",$dbtrack);

	// CHECK FOR EXACT MATCH

    $sqlsh = "SELECT COUNT(*) as TotalFound FROM tracks WHERE TrackName LIKE '$Search[1]' ORDER BY TrackName";
	$resultsh = mysql_query($sqlsh,$dbtrack);
	if ($myrowsh = mysql_fetch_array($resultsh)) {

	    if ($myrowsh["TotalFound"]==1) {

		$sql = "SELECT * FROM tracks WHERE TrackName LIKE '$Search[1]' ORDER BY TrackName";
		$result = mysql_query($sql,$dbtrack);
		$myrow = mysql_fetch_array($result);

	    } else {
	        $DoPartial=TRUE;
	    }

	}
	if ($DoPartial) {

		$sqlsha = "SELECT COUNT(*) as TotalFound FROM tracks WHERE TrackName LIKE '%$Search[1]%' ORDER BY TrackName LIMIT 20";
		$resultsha = mysql_query($sqlsha,$dbtrack);
		if ($myrowsha = mysql_fetch_array($resultsha)) {

			if ($myrowsha["TotalFound"]==1) {
				$sql = "SELECT * FROM tracks WHERE TrackName LIKE '%$Search[1]%' ORDER BY TrackName";
				$result = mysql_query($sql,$dbtrack);
				$myrow = mysql_fetch_array($result);
			} elseif ($myrowsha["TotalFound"]>1) {
	    		$result = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','$addchatid[0]','IMR','Your search \'$Search[1]\' provided too many results, be more ','".time()."')",$db);
	    		sleep(1);
	    		$result = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','$addchatid[0]','IMR','specific. Here is a list of the results returned:','".time()."')",$db);
	    		sleep(1);
	    		$sql = "SELECT * FROM tracks WHERE TrackName LIKE '%$Search[1]%' ORDER BY TrackName LIMIT 20";
				$result = mysql_query($sql,$dbtrack);
				if ($myrow = mysql_fetch_array($result)) {
		    		do {
            			$myrow["TrackName"] = str_replace("'", "\'", $myrow["TrackName"]);
		                $resultn = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','$addchatid[0]','','$myrow[TrackName]','".time()."')",$db);
	    				sleep(1);
		    		} while ($myrow = mysql_fetch_array($result));
				}
			} elseif ($myrowsha["TotalFound"]==0) {
				$result = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','$addchatid[0]','IMR','Your search provided no results.','".time()."')",$db);
				sleep(1);
			}

		}

	}

	    if ($myrow["Tracksum"]) {
		$sqlshow = "SELECT * FROM records WHERE TrackSum=$myrow[Tracksum] ORDER BY LapTime DESC LIMIT 1";
		$resultshow = mysql_query($sqlshow,$dbtrack);
		$myrowshow = mysql_fetch_array($resultshow);
		if ($myrowshow["LapTime"]) {
		$resultmsg = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','$addchatid[0]','','Lap Records for \'$myrow[TrackName]\' are:','".time()."')",$db);
		} else {
		$resultmsg = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','$addchatid[0]','','No records have been set for \'$myrow[TrackName]\'.','".time()."')",$db);
		}
		sleep(1);
		// SHOW BASIC
	    $sqlshow = "SELECT * FROM records WHERE TrackSum=$myrow[Tracksum] and CraftModel=0 ORDER BY LapTime LIMIT 1";
		$resultshow = mysql_query($sqlshow,$dbtrack);
		$myrowshow = mysql_fetch_array($resultshow);
		if ($myrowshow["LapTime"]) {
			$mins = floor(substr($myrowshow["LapTime"],0,strlen($myrowshow["LapTime"])-3) / 60);
			$secs = substr($myrowshow["LapTime"],0,strlen($myrowshow["LapTime"])-3) % 60;
			if ($secs < 10) { $secs = "0" . $secs;}
			$milli = substr($myrowshow["LapTime"],-3,3);
			$resultmsg = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','$addchatid[0]','Basic','$mins:$secs.$milli - $myrowshow[Alias]','".time()."')",$db);
		}
		sleep(1);
		// SHOW BI
	    $sqlshow = "SELECT * FROM records WHERE TrackSum=$myrow[Tracksum] and CraftModel=2 ORDER BY LapTime LIMIT 1";
		$resultshow = mysql_query($sqlshow,$dbtrack);
		$myrowshow = mysql_fetch_array($resultshow);
		if ($myrowshow["LapTime"]) {
			$mins = floor(substr($myrowshow["LapTime"],0,strlen($myrowshow["LapTime"])-3) / 60);
			$secs = substr($myrowshow["LapTime"],0,strlen($myrowshow["LapTime"])-3) % 60;
			if ($secs < 10) { $secs = "0" . $secs;}
			$milli = substr($myrowshow["LapTime"],-3,3);
			$resultmsg = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','$addchatid[0]','BI-T»','$mins:$secs.$milli - $myrowshow[Alias]','".time()."')",$db);
		}
		sleep(1);
		// SHOW CX
	    $sqlshow = "SELECT * FROM records WHERE TrackSum=$myrow[Tracksum] and CraftModel=1 ORDER BY LapTime LIMIT 1";
		$resultshow = mysql_query($sqlshow,$dbtrack);
		$myrowshow = mysql_fetch_array($resultshow);
		if ($myrowshow["LapTime"]) {
			$mins = floor(substr($myrowshow["LapTime"],0,strlen($myrowshow["LapTime"])-3) / 60);
			$secs = substr($myrowshow["LapTime"],0,strlen($myrowshow["LapTime"])-3) % 60;
			if ($secs < 10) { $secs = "0" . $secs;}
			$milli = substr($myrowshow["LapTime"],-3,3);
			$resultmsg = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','$addchatid[0]','CX»»','$mins:$secs.$milli - $myrowshow[Alias]','".time()."')",$db);
		}
		sleep(1);
		// SHOW EON
	    $sqlshow = "SELECT * FROM records WHERE TrackSum=$myrow[Tracksum] and CraftModel=7 ORDER BY LapTime LIMIT 1";
		$resultshow = mysql_query($sqlshow,$dbtrack);
		$myrowshow = mysql_fetch_array($resultshow);
		if ($myrowshow["LapTime"]) {
			$mins = floor(substr($myrowshow["LapTime"],0,strlen($myrowshow["LapTime"])-3) / 60);
			$secs = substr($myrowshow["LapTime"],0,strlen($myrowshow["LapTime"])-3) % 60;
			if ($secs < 10) { $secs = "0" . $secs;}
			$milli = substr($myrowshow["LapTime"],-3,3);
			$resultmsg = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','$addchatid[0]','EoN»','$mins:$secs.$milli - $myrowshow[Alias]','".time()."')",$db);
		}
	    }
	    ?>
