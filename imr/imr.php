<?

/*
------AUSTINS PHP IMR------

MYSQL Database Construction

table : chat

layout: MessageID               - BIGINT
		UserID					- INT
		UserName                - VARCHAR[40]
		Message                 - VARCHAR[255]
		TimeStamp               - VARCHAR[25]

--

table : debug

layout:	string					- VARCHAR[100]

--

table : gamelist

layout: GameID                  - INT
		UserID					- INT
		GameName                - VARCHAR[50]
		Track                   - VARCHAR[50]
		Laps                    - INT
		Weapons                 - INT
		GameIP                  - VARCHAR[15]
		Port                    - INT

--

table : gameplayers

layout: GameID                  - INT
		UserID					- INT
		JoinOrder				- INT

--

table : olduserlist

layout: UserName				- VARCHAR[40]
		IPAddress               - VARCHAR[15]
		TimeStamp				- VARCHAR[25]

--

table : userlist

layout: UserID					- INT
		UserName                - VARCHAR[40]
		RegKey                  - VARCHAR[8]
		Version                 - VARCHAR[10]
		Key2	                - VARCHAR[10]
		Key3					- VARCHAR[10]
		TimeStamp				- VARCHAR[25]
		MessageTimeStamp		- VARCHAR[25]
		JoinedIMR               - VARCHAR[25]
		PlayerListUpdate        - VARCHAR[25]
		StartedGame             - VARCHAR[25]
		GameListUpdate			- VARCHAR[25]

--

Hope you get some use out of this!

-Austin
---------------------------
*/

// MYSQL CONFIG FILE : dbconnect.php
include "dbconnect.php";

// CURRENT TIME FOR IMR SHOWING (Comment out to not include this
$currenttime = "<" . date("H:i:s") . "> ";

// IF MYSQL IS BUSY, DISPLAY A MESSAGE IN THE IMR
$link = mysql_connect("$serverip", "$serverusername", "$serverpassword");
if (mysql_errno() == 1203) {
  // 1203 == ER_TOO_MANY_USER_CONNECTIONS (mysqld_error.h)
  print "CHAT\nIMR» There are currently too many connections to the database, hold tight, once ";
  print "these message stops repeating itself, the IMR should be fine.\n";
  exit;
}

$db = mysql_connect($serverip, $serverusername, $serverpassword);
mysql_select_db($databasename,$db);

$maxclient = 50; 			// MOST AMOUNT OF PLAYERS ALLOWED IN THE IMR
$maxplayersingame = 10; 	// MOST AMOUNT OF PLAYERS ALLOWED IN THE GAME
$maxgames =  15; 			// MOST AMOUNT OF GAMES ALLOWED IN THE IMR
$timeout   = time();
$timeout   -= 15;			// HOW MANY SECONDS BEFORE A PLAYER HAS TIMED OUT
$chatrem   = time();
$chatrem   -= 30;			// HOW MANY SECONDS BEFORE CHAT IS REMOVED FROM THE DATABASE
$olduserrem= time();
$olduserrem-= 60*60*24;     // HOW MANY SECONDS TO REMOVE OLD USERS FROM THE OLDUSERLIST TABLE (60*60*24=A DAY)

$weburl="http://www.hoverrace.com";     // WEBSITE ADDRESS

// GETTING THEIR IP
$playerip = isset($_SERVER['HTTP_X_FORWARDED_FOR']) ? $_SERVER['HTTP_X_FORWARDED_FOR'] : $_SERVER['REMOTE_ADDR'];

// DEBUG ERRORS WITH : $result = mysql_query("INSERT INTO debug (string) VALUES ('$_SERVER[QUERY_STRING]')",$db);

// DISECTING THE QUERY STRING FOR FUTURE USAGE

$command = split("%%", $_SERVER["QUERY_STRING"]);

// UPDATE THE GAMELEAVE TABLE'S "PLAYERNAMES" COLUMN TO REFLECT YOUR LEAVING
function RefreshPlayerNames($gameid)
{
    include "dbconnect.php";
    $db = mysql_connect($serverip, $serverusername, $serverpassword);
	mysql_select_db($databasename,$db);
	
    // LOOP THE PLAYERS STILL IN THE GAME
    $newpn = "";
    $resultgameleaveadd = mysql_query("SELECT * FROM gameplayers WHERE GameID='$gameid' AND JoinOrder>1",$db);
    if ($myrowleave = mysql_fetch_array($resultgameleaveadd))
    {
        do
		{
		    // GET THE PLAYERS NAME AND ADD THEM TO THE ARRAY
		    $resultgameleavename = mysql_query("SELECT * FROM userlist WHERE UserID='$myrowleave[UserID]'",$db);
    		$myrowname = mysql_fetch_array($resultgameleavename);
    		$newpn = $newpn . "[$myrowleave[JoinOrder]]$myrowname[UserName]<$myrowleave[JoinOrder]>";
		} while ($myrowleave = mysql_fetch_array($resultgameleaveadd));
    }
    // UPDATE "PLAYERNAMES"
    $resultgameplayers = mysql_query("UPDATE gameleave SET PlayerNames='$newpn' WHERE GameID='$gameid'",$db);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// REFRESHING THE IMR                                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

if ($command[0]=="=REFRESH") {
	// UPDATE AND PRINT USERLIST
	// STRIP - OFF OF THE USERID
    $refreshid = split("-",$command[1]);

        // CHECK IF THEY STILL EXIST ON THE USERLIST OR KICK THEM OUT OF THE ROOM
        $result = mysql_query("SELECT * FROM userlist WHERE UserID=$refreshid[0]",$db);
		if ($myrow = mysql_fetch_array($result)) {} else {
		    print "ERROR 201\n" ;
			exit;
		}

		// CHECK IF THERE IS ANY NEED TO UPDATE THE USERLIST BY CHECKING THE TIMESMAMP OF THE LAST PERSON JOINED
		// AGAINST THE LAST TIMESTAMP OF YOU REFRESHING THE USERLIST
		$resultcts = mysql_query("SELECT * FROM userlist ORDER BY JoinedIMR DESC LIMIT 1",$db);
		$myrowcts = mysql_fetch_array($resultcts);
    	$resultmts = mysql_query("SELECT * FROM userlist WHERE UserID=$refreshid[0] and PlayerListUpdate<$myrowcts[JoinedIMR]",$db);
   		if ($myrowmts = mysql_fetch_array($resultmts)) {
 		// CHECKS IF ANY OF THE CLIENT SLOTS ARE TAKEN
    	for ( $clientcount=1; $clientcount <= $maxclient; $clientcount++) {
    	    $result = mysql_query("SELECT * FROM userlist WHERE UserID=$clientcount and TimeStamp >$timeout",$db);
    		if ($myrow = mysql_fetch_array($result)) {
    		    // TELLS THE IMR TO ADD THIS CLIENT TO THE PLAYER LIST
            	print "USER $myrow[UserID] NEW\n";
            	print "\n";
            	print urldecode($myrow["UserName"]);
            	if ($myrow["Status"]==TRUE) {
            	    print " [" . urldecode($myrow[Status]) . "]";
            	}
				print "\n";
    		} else {
    		    // TELLS THE IMR THERE IS NOBODY IN THIS CLIENT SLOT
    		    print "USER $clientcount DEL\n";
    		}
    	}
    	// UPDATES THE TIMESTAMP OF WHEN YOU LAST UPDATED THE USERLIST TO THE SAME
		// AS THE LAST PERSON TO JOIN THE ROOM
    	$result = mysql_query("UPDATE userlist SET PlayerListUpdate=$myrowcts[JoinedIMR] WHERE UserID=$refreshid[0]",$db);
 		}
 		// UPDATE THE PLAYERS TIMESTAMP SHOWING HE IS ACTIVE IN THE IMR
    	$result = mysql_query("UPDATE userlist SET TimeStamp=" . time() . " WHERE UserID=$refreshid[0]",$db);

		// CHECK IF THEY TIMED OUT AND REMOVE THEM FROM THE DATABASE
		$resulttimeout = mysql_query("SELECT * FROM userlist WHERE TimeStamp<$timeout",$db);
    	if ($myrowtimeout = mysql_fetch_array($resulttimeout)) {
			do
			{
    			$result = mysql_query("INSERT INTO chat (UserID,UserName,Message,TimeStamp) VALUES ('0','$currenttime','$myrowtimeout[UserName] has timed out!',". time() .")",$db);
    			$result = mysql_query("DELETE FROM userlist WHERE UserID='$myrowtimeout[UserID]' LIMIT 1",$db);
    			$result = mysql_query("DELETE FROM gameplayers WHERE UserID='$myrowtimeout[UserID]' LIMIT 1",$db);
    			$result = mysql_query("DELETE FROM gamelist WHERE UserID='$myrowtimeout[UserID]' LIMIT 1",$db);
				// REMOVE THEM FROM HOSTING A GAME
				$resultleaveh = mysql_query("SELECT * FROM gameleave WHERE HostName='$myrowtimeout[UserName]'",$db);
    			if ($myrowleaveh = mysql_fetch_array($resultleaveh)) {
    			    $result = mysql_query("DELETE FROM gameleave WHERE HostName='$myrowtimeout[UserName]'",$db);
    			}
    			$resultleavej = mysql_query("SELECT * FROM gameleave WHERE PlayerNames LIKE '%$myrowtimeout[UserName]%'",$db);
    			if ($myrowleavej = mysql_fetch_array($resultleavej)) {
           			RefreshPlayerNames($myrowleavej["GameID"]);
    			}
    		} while ($myrowtimeout = mysql_fetch_array($resulttimeout));
    		// FORCES A RELOAD ON THE USERLIST AND GAMELIST
    		$result = mysql_query("UPDATE userlist SET PlayerListUpdate=0, GameListUpdate=0",$db);
		}

		// REMOVE USERS IN THE OLDUSERLIST TABLE
		$result = mysql_query("DELETE FROM olduserlist WHERE TimeStamp<$olduserrem",$db);

		// CHECKS WHEN THE CLIENT LAST UPDATED THE GAME LIST AGAINST WHEN THE LAST GAME WAS CREATED
        $resultcts = mysql_query("SELECT * FROM userlist ORDER BY StartedGame DESC LIMIT 1",$db);
		$myrowcts = mysql_fetch_array($resultcts);
    	$resultmts = mysql_query("SELECT * FROM userlist WHERE UserID=$refreshid[0] and GameListUpdate<$myrowcts[StartedGame]",$db);
 		if ($myrowmts = mysql_fetch_array($resultmts)) {
	 		// PRINT GAMES TO THE GAMES LIST
	    	for ( $gamescount=1; $gamescount <= $maxgames; $gamescount++) {
	    	    $result = mysql_query("SELECT * FROM gamelist WHERE GameID=$gamescount",$db);
	    		if ($myrow = mysql_fetch_array($result)) {
	    		    // COUNTS THE PLAYERS IN THE GAME
	    		    $resultpc = mysql_query("SELECT COUNT(*) AS PlayerCount FROM gameplayers WHERE GameID=$gamescount",$db);
	    		    $myrowpc = mysql_fetch_array($resultpc);
	    		    // TELLS THE IMR TO ADD THIS GAME TO THE GAME LIST
	            	print "GAME $myrow[GameID] NEW $myrow[UserID]\n";
	            	print stripslashes(urldecode($myrow["GameName"])) . "\n";
	            	print stripslashes(urldecode($myrow["Track"])) . "\n";
	            	print "$myrow[GameIP]\n";
	            	print "$myrow[Port] $myrow[Laps] $myrow[Weapons] $myrowpc[PlayerCount]\n";
	            	// WRITES THE PLAYER'S ID'S INTO SOME FORM OF ARRAY FOR THE PLAYERLIST
	                $resultshowpl = mysql_query("SELECT * FROM gameplayers WHERE GameID=$gamescount ORDER BY JoinOrder",$db);
	            	if ($myrowshowpl = mysql_fetch_array($resultshowpl)) {
	            	    do {
	            			print "$myrowshowpl[UserID] ";
	            	    } while ($myrowshowpl = mysql_fetch_array($resultshowpl));
	            	}
	            	print "\n" ;
	            	
	            	if($myrow["UserID"]==$refreshid[0] AND $myrow["Checked"]=="0")
					{
					    $reloadgames = TRUE;
						file_get_contents("http://" . $_SERVER["SERVER_ADDR"] . $_SERVER['PHP_SELF'] . "?=CHECK_GAME%%$myrow[UserID]%%$myrow[GameIP]%%$myrow[Port]%%$myrow[GameID]");
						$result = mysql_query("UPDATE gamelist SET Checked=1 WHERE UserID=$myrow[UserID] AND GameID=$myrow[GameID]",$db);
					}
	    		} else {
	    		    print "GAME $gamescount DEL\n";
	    		}
	    	}
	    	// UPDATES THE TIMESTAMP OF WHEN YOU LAST UPDATED THE GAMELIST TO THE SAME
			// AS THE LAST TIME A GAME WAS UPDATED
	    	if ($reloadgames==FALSE) $result = mysql_query("UPDATE userlist SET GameListUpdate=$myrowcts[StartedGame] WHERE UserID=$refreshid[0]",$db);
 		}

	// CHECK IF THERE ARE ANY NEW MESSAGES TO DISPLAY
	$resultmts = mysql_query("SELECT * FROM userlist WHERE UserID=$refreshid[0]",$db);
 	$myrowmts = mysql_fetch_array($resultmts);
	$resultcts = mysql_query("SELECT * FROM chat ORDER BY MessageID DESC LIMIT 1",$db);
	$myrowcts = mysql_fetch_array($resultcts);
	// PRINT NEW MESSAGES
	if ($myrowcts["TimeStamp"]>$myrowmts["MessageID"]) {
        $resultshowmess = mysql_query("SELECT * FROM chat WHERE MessageID>$myrowmts[MessageTimeStamp] ORDER BY TimeStamp",$db);
        if ($myrowshowmess = mysql_fetch_array($resultshowmess)) {
        	do {
        	    // SEND A PRIVATE MESSAGE
        	    if ($myrowshowmess["ToUser"]==TRUE and $refreshid[0]==$myrowshowmess["ToUser"]) {
				print "CHAT\n" . urldecode($myrowshowmess["UserName"]) . " <private>» ";
                $message = $myrowshowmess["Message"];
				$message = str_replace("+","%2B",$message);
				$message = str_replace("\'","'",$message);
				print urldecode($message). "\n";
        	    } elseif ($myrowshowmess["ToUser"]==FALSE) {
        	    // PRINTS OFF THE NEW MESSAGE AND WHO IT IS BY
             	print "CHAT\n" . urldecode($myrowshowmess["UserName"]) . "» ";
                $message = $myrowshowmess["Message"];
				$message = str_replace("+","%2B",$message);
				$message = str_replace("\'","'",$message);
				print urldecode($message). "\n";
        	    }
        	} while ($myrowshowmess = mysql_fetch_array($resultshowmess));
        }
        $result = mysql_query("UPDATE userlist SET MessageTimeStamp=$myrowcts[MessageID] WHERE UserID=$refreshid[0]",$db);
	}

 	// AFTER WHATEVER THE CHAT REMOVAL IS SET TO IN SECONDS, THE CHAT MESSAGES ARE DELETED FROM THE DATABASE
 	// TO SAVE IT BECOMING HUGE
  	// $result = mysql_query("DELETE FROM chat WHERE TimeStamp<$chatrem ORDER BY TimeStamp DESC OFFSET 10",$db);

  	print "SUCCESS\n";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ADDING THE USER 																							 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

if ($command[0]=="=ADD_USER") {

    // CHECK FOR OLD CLIENTS AND REMOVE THEM, ALSO STOPS IMR FILLING
   	$result = mysql_query("DELETE FROM userlist WHERE TimeStamp<$timeout",$db);
   	// CHECK FOR SAME NAME AND ADD A NUMBER ONTO THE END IF THERE IS A SAME NAME IN THE IMR
   	$result = mysql_query("SELECT * FROM userlist WHERE UserName='$command[5]'",$db);
   	if ($myrow = mysql_fetch_array($result)) {
  		do {
  		    $count++;
  		    $alteredcommand5 = $command[5] . $count;
            $resultcheck = mysql_query("SELECT * FROM userlist WHERE UserName='$alteredcommand5'",$db);
            if (($myrowcheck = mysql_fetch_array($resultcheck))==FALSE) {
                $UniqueName=TRUE;
            }
		} while ($UniqueName==FALSE);
		$command[5]=$alteredcommand5;
   	}
	// CHECK HOW MANY PEOPLE ARE IN THE IMR AND ASSIGN A UNIQUE USERID
	do {
	$UserID++;
	$result = mysql_query("SELECT * FROM userlist WHERE UserID=$UserID",$db);
    if ($myrow = mysql_fetch_array($result)==FALSE) {
        $OriginalID=TRUE;
    }
	} while ($OriginalID==FALSE) ;
	// GIVE AN ERROR IF THERE ARE ALREADY THE MAXIMUM AMOUNT OF CLIENTS IN THE ROOM
	if ($UserID > $maxclient) {
		print "ERROR 102\n" ;
		exit;
	}
	// TELL THE CLIENT THAT JOINING WAS A SUCCESS
	print "SUCCESS\n";
	print "USER_ID $UserID\n";
	// RETRIEVE THE TIMESTAMP OF THE LAST MESSAGE ON THE IMR, SO THAT THE REFRESH COMMAND PICKS UP NEW MESSAGES
	$resultmts = mysql_query("SELECT * FROM chat ORDER BY TimeStamp DESC LIMIT 1",$db);
    if ($myrowmts = mysql_fetch_array($resultmts)) {
        $MessageTimeStamp=$myrowmts["MessageID"];
    } else {
        // IF THE CHAT DATABASE IS EMPTY, IT GIVES THE MESSAGE TIMESTAMP A "1" VALUE
        $MessageTimeStamp=1;
    }
	// INSERT USER INTO DATABASE CHECKING FOR HIS OLD IP ADDRESS SETTING IN OUR OLDUSERLIST TABLE
	$resultip = mysql_query("SELECT * FROM olduserlist WHERE UserName='$command[5]'",$db);
    if ($myrowip = mysql_fetch_array($resultip)) {
        // TELL THE IMR A NEW PLAYER HAS JOINED
    	$result = mysql_query("INSERT INTO chat (UserID,UserName,Message,TimeStamp) VALUES ('0','$currenttime','$command[5] has joined the IMR! ($myrowip[IPAddress])',". time() .")",$db);
        $result = mysql_query("INSERT INTO userlist (UserName,UserID,RegKey,Version,Key2,Key3,IPAddress,IPAddressOriginal,TimeStamp,MessageTimeStamp,JoinedIMR,PlayerListUpdate,GameListUpdate,StartedGame) VALUES ('$command[5]','$UserID','$command[1]','$command[2]','$command[3]','$command[4]','$myrowip[IPAddress]','$playerip','". time() ."',$MessageTimeStamp,'". time() ."','". time() ."','1','1')",$db);
        $result = mysql_query("DELETE FROM olduserlist WHERE UserName='$command[5]'",$db);
    } else {
        // TELL THE IMR A NEW PLAYER HAS JOINED
    	$result = mysql_query("INSERT INTO chat (UserID,UserName,Message,TimeStamp) VALUES ('0','$currenttime','$command[5] has joined the IMR! ($playerip)',". time() .")",$db);
        $result = mysql_query("INSERT INTO userlist (UserName,UserID,RegKey,Version,Key2,Key3,IPAddress,IPAddressOriginal,TimeStamp,MessageTimeStamp,JoinedIMR,PlayerListUpdate,GameListUpdate,StartedGame) VALUES ('$command[5]','$UserID','$command[1]','$command[2]','$command[3]','$command[4]','$playerip','$playerip','". time() ."',$MessageTimeStamp,'". time() ."','". time() ."','1','1')",$db);
    }

	// DOES AN INITIAL PLAYER AND GAME LIST REFRESH TO UPDATE THE USERLIST

	// CHECKS IF ANY OF THE CLIENT SLOTS ARE TAKEN
    	for ( $clientcount=1; $clientcount <= $maxclient; $clientcount++) {
    	    $result = mysql_query("SELECT * FROM userlist WHERE UserID=$clientcount and TimeStamp>$timeout",$db);
    		if ($myrow = mysql_fetch_array($result)) {
    		    // TELLS THE IMR TO ADD THIS CLIENT TO THE PLAYER LIST
            	print "USER $myrow[UserID] NEW\n";
            	print "\n";
            	print urldecode($myrow["UserName"]);
                if ($myrow["Status"]==TRUE) {
            	    print " [" . urldecode($myrow[Status]) . "]";
            	}
				print "\n";
    		} else {
    		    // TELLS THE IMR THERE IS NOBODY IN THIS CLIENT SLOT
    		    print "USER $clientcount DEL\n";
    		}
    	}

    // PRINT GAMES TO THE GAMES LIST

    	for ( $gamescount=1; $gamescount <= $maxgames; $gamescount++) {
    	    $result = mysql_query("SELECT * FROM gamelist WHERE GameID=$gamescount",$db);
    		if ($myrow = mysql_fetch_array($result)) {
    		    // COUNTS THE PLAYERS IN THE GAME
    		    $resultpc = mysql_query("SELECT COUNT(*) AS PlayerCount FROM gameplayers WHERE GameID=$gamescount",$db);
    		    $myrowpc = mysql_fetch_array($resultpc);
    		    // TELLS THE IMR TO ADD THIS GAME TO THE GAME LIST
            	print "GAME $myrow[GameID] NEW $myrow[UserID]\n";
            	print urldecode($myrow["GameName"]) . "\n";
            	print urldecode($myrow["Track"]) . "\n";
            	print "$myrow[GameIP]\n";
            	print "$myrow[Port] $myrow[Laps] $myrow[Weapons] $myrowpc[PlayerCount]\n";
                $resultshowpl = mysql_query("SELECT * FROM gameplayers WHERE GameID=$gamescount ORDER BY JoinOrder",$db);
            	if ($myrowshowpl = mysql_fetch_array($resultshowpl)) {
            	    do {
            			print "$myrowshowpl[UserID] ";
            	    } while ($myrowshowpl = mysql_fetch_array($resultshowpl));
            	}
            	print "\n" ;
    		} else {
    		    print "GAME $gamescount DEL\n";
    		}
    	}
	// ADDS TO THE SMALL INTRO MESSAGE
    print "CHAT\nWritten in PHP by Austin Brock [ sir_brock@hotmail.com ]\n";
    print "CHAT\n--\n";
    // PRINT THE INTRO MESSAGE IF ONE EXISTS
    $resultint = mysql_query("SELECT * FROM intro",$db);
    if ($myrowint = mysql_fetch_array($resultint)) {
        $message = $myrowint["Message"];
        $message = str_replace("\n", "\nCHAT\n", $message);
        print "CHAT\n$message\n";
        print "CHAT\n--\n";
    }


}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// REMOVING THE USER FROM THE IMR 																			 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

if ($command[0]=="=DEL_USER") {
	// REMOVE THE CLIENT FROM THE IMR AS HE LEAVES
	$leaveid = split("-",$command[1]);
	$resultmts = mysql_query("SELECT * FROM userlist WHERE UserID=$leaveid[0]",$db);
	$myrowmts = mysql_fetch_array($resultmts);
	// ADD THEM TO A STORED DATABASE SO THAT THEIR IP ADDRESS IS REMEMBERED IF THEY HAVE CHANGED IT
	if ($myrowmts["IPAddress"]<>$myrowmts["IPAddressOriginal"]) {
		$result = mysql_query("INSERT INTO olduserlist (UserName,IPAddress,TimeStamp) VALUES ('$myrowmts[UserName]','$myrowmts[IPAddress]',". time() .")",$db);
	}
	
	// IF HE LEFT TO RACE, PUT HIM INTO A TABLE CONTAINING OTHER PLAYERS WHO JUST LEFT
	$resulthost = mysql_query("SELECT * FROM gamelist WHERE UserID='$leaveid[0]'",$db);
	$resultjoin = mysql_query("SELECT * FROM gameplayers WHERE UserID='$leaveid[0]'",$db);
	if ($myrowhost = mysql_fetch_array($resulthost))
	{
	    // PRINT THE MESSAGE SAYING PEOPLE ARE LEAVING TO PLAY
	    $resultleave = mysql_query("SELECT * FROM gameleave WHERE GameID='$myrowhost[GameID]'",$db);
	    $myrowleave = mysql_fetch_array($resultleave);
	    $result = mysql_query("INSERT INTO chat (UserID,UserName,Message,TimeStamp) VALUES ('0','$currenttime', '" . urldecode($myrowleave["HostName"]) . " has launched a game.',". time() .")",$db);
		if ($myrowleave["Weapons"]==1) { $weapons = " with weapons."; } else { $weapons = " and no weapons."; }
		$result = mysql_query("INSERT INTO chat (UserID,UserName,Message,TimeStamp) VALUES ('0','Game Details','$myrowleave[GameName], $myrowleave[Laps] laps$weapons',". time() .")",$db);
	    // NOW PRINT THE PARTICIPANTS
	    $players = "<#1>" . urldecode($myrowleave["HostName"]);
	    // NOW ADD THE OTHERS IN ORDER
		$extraplayers = " " . $myrowleave["PlayerNames"];
		if ($extraplayers != " ")
		{
			for ($playersingame=2; $playersingame<=$maxplayersingame; $playersingame++)
			{
			    if (stripos($extraplayers, "[" . $playersingame . "]"))
			    {
			    	$splitplayer = split("\[$playersingame\]", $extraplayers);
			    	$splitplayer = split("\<$playersingame\>", $splitplayer[1]);
			    	$splitplayer = $splitplayer[0];
			    	$players .= ", <#$playersingame>" . urldecode($splitplayer);
				}
			}
		}
	    $result = mysql_query("INSERT INTO chat (UserID,UserName,Message,TimeStamp) VALUES ('0','Game Players','$players',". time() .")",$db);
	    $resultb = mysql_query("DELETE FROM gameleave WHERE GameID='$myrowhost[GameID]'",$db);
	} elseif (($myrowjoin = mysql_fetch_array($resultjoin))==FALSE) {
		// TELL THE ROOM HE HAS LEFT
		$result = mysql_query("INSERT INTO chat (UserID,UserName,Message,TimeStamp) VALUES ('0','$currenttime','$myrowmts[UserName] has left the IMR!',". time() .")",$db);
	}
	
	// REMOVE HIM FROM THE USERLIST
  	$result = mysql_query("DELETE FROM userlist WHERE UserID='$leaveid[0]' LIMIT 1",$db);
  	// REMOVE HIM FROM ANY GAMES (BECAUSE WHEN YOU START A GAME, THEY ONLY LEAVE THE ROOM)
  	$result = mysql_query("DELETE FROM gamelist WHERE UserID='$leaveid[0]'",$db);
    $resultb = mysql_query("DELETE FROM gameplayers WHERE UserID='$leaveid[0]'",$db);
    // FORCES A RELOAD ON THE USERLIST AND GAMELIST (BECAUSE WHEN YOU START A GAME, THEY ONLY LEAVE THE ROOM)
    $resultc = mysql_query("UPDATE userlist SET PlayerListUpdate='0', GameListUpdate='0'",$db);
	print "SUCCESS\n";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ADDING CHAT TO THE CHAT DATABASE 																		 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

if ($command[0]=="=ADD_CHAT") {
    // STRIP - OFF OF THE USERID AND UPDATE THE TIMESTAMP
    $addchatid = split("-",$command[1]);
    $result = mysql_query("SELECT * FROM userlist WHERE UserID=$addchatid[0]",$db);
    if ($myrow = mysql_fetch_array($result)) {
        $UserName = $myrow["UserName"];
    }

    if (substr(urldecode($command[2]),0,3)=="/ip") {
    	// LET A PLAYER CHANGE HIS IP (USEFUL IF YOU WANT TO USE HAMACHI)
    	/////////////////////////////////////////////////////////////////
    	$changedip = split ("/ip ",urldecode($command[2]));
		$resultmts = mysql_query("SELECT * FROM userlist WHERE UserID=$addchatid[0]",$db);
		$myrowmts = mysql_fetch_array($resultmts);
		// IF NO IP IS GIVEN, SIMPLY CHANGE TO THEIR ORIGINAL IP
		if ($changedip[1]==FALSE) {
	    	$changedip[1] = $playerip;
		}
		if (is_numeric(str_replace(".", "", $changedip[1]))) {
		    $result = mysql_query("UPDATE userlist SET IPAddress='$changedip[1]' WHERE UserID=$addchatid[0]",$db);
		    $result = mysql_query("UPDATE gamelist SET GameIP='$changedip[1]' WHERE UserID=$addchatid[0]",$db);
    		$result = mysql_query("INSERT INTO chat (UserID,UserName,Message,TimeStamp) VALUES ('0','$currenttime','$myrowmts[UserName] has changed IP to: $changedip[1]','".time()."')",$db);
		} else {
		    $result = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','$addchatid[0]','$currenttime','You cannot change your IP to text.','".time()."')",$db);
		}
	} elseif (substr(urldecode($command[2]),0,4)=="/msg") {
		// SENDING PRIVATE MESSAGES
		///////////////////////////

		// SPLIT THE USER/MESSAGE BIT AWAY
    	$split = split ("msg%20",$command[2],2);
    	// SPLIT THE USER AND MESSAGE INTO AN ARRAY
    	$contentsmessage = split ("%20","$split[1]", 2);
    	$contents = split ("%20","$split[1]");
    	// CHECK IF THE USER EXISTS AND DETERMINES IF TO SEND THE MESSAGE OR TELL THE PLAYER HE DOESN'T EXIST
    	$undel = 5;
    	// IT TRIES 5 TIMES TO FIND THE USERS NAME, BUT THE WAY IT DOES IT, IS THAT IT CHECKS THE FIRST NAME BETWEEN SPACES, AND SEES IF ITS IN THE TABLE
    	// IF THE PLAYERS NAME WAS "JOE", IT WOULD FIND IT FIRST TIME.  IF THEIR NAME WAS "JOE BLOGGS", IT SHOULD FIND IT ON THE SECOND OCCASION
    	// IF A PLAYER CAME INTO THE ROOM CALLED "JOE" WHILE "JOE BLOGGS" WAS IN THE ROOM, UNFORTUNATELY IT WOULD FIND JOE FIRST.
    	//
    	// SO I WILL BE DOING IT IN REVERSE, IF THE MESSAGE TO JOE WAS "HI JOE, HOW ARE YOU?", IT WOULD SEARCH FOR THE USERNAME "JOE HI JOE, HOW ARE YOU?"
    	// AND THEN REVERSE THE PROCESS, NEXT NAME WOULD BE "JOE HI JOE, HOW ARE", UNTIL IT GOT TO "JOE".  IT IS MUCH MORE UNLIKELY THAT A MESSAGE TO JOE
    	// WOULD BE "BLOGGS SAID HE DISLIKES YOU", SENDING THE MESSAGE TO "JOE BLOGGS" INSTEAD.  BUT THIS WOULD MEAN THE SENDER WAS AN IDIOT :).
    	do {
    	    $name = FALSE;
     		for ( $x = 0; $x <= $undel; $x+=1 ) {
     		    if ($x > 0) {
     		    	$name .= "%20";
     		    }
     			$name .= $contents[$x];
     		}
       		$resultmtsb = mysql_query("SELECT * FROM userlist WHERE UserName LIKE '". $name ."'",$db);
			if ($myrowmtsb = mysql_fetch_array($resultmtsb)) {
	    		// DELIVER THE MESSAGE
	    		$message = str_replace("$name%20", "", "$contentsmessage[0]%20$contentsmessage[1]");
    			$result = mysql_query("INSERT INTO chat (UserID,ToUser,ToUserName,UserName,Message,TimeStamp) VALUES ('$addchatid[0]','$myrowmtsb[UserID]','$name','$currenttime$UserName','". urldecode(addslashes($message)) ."','".time()."')",$db);
    			$result = mysql_query("INSERT INTO chat (UserID,ToUser,ToUserName,UserName,Message,TimeStamp) VALUES ('$addchatid[0]','$addchatid[0]','$name','$currenttime$UserName','". urldecode(addslashes($message)) ."','".time()."')",$db);
    			$undel=-10;
			}
			$undel--;
    	} while ($undel>-1);
		if ($undel==-1) {
	    	// TELL THEM THE PLAYER DOESN'T EXIST
	    	$result = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','$addchatid[0]','$currenttime\IMR','This player does not exist.','".time()."')",$db);
		}
	} elseif (substr(urldecode($command[2]),0,4)=="/rec") {
		// VIEWING RECORDS FOR HOSTED TRACK
		///////////////////////////////////
		include "command-rec.php";
	// CODE FOR A STATUS CHANGE
	} elseif (substr(urldecode($command[2]),0,7)=="/status") {
		// LET A PLAYER CHANGE HIS STATUS
    	/////////////////////////////////
    	$changedstatus = split ("status%20",$command[2]);
		$resultname = mysql_query("SELECT * FROM userlist WHERE UserID=$addchatid[0]",$db);
		$myrowname = mysql_fetch_array($resultname);
		if ($changedstatus[1]) {
			$result = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','0','$currenttime','$myrowname[UserName] has changed their status to \'$changedstatus[1]\'','".time()."')",$db);
		} elseif ($myrowname["Status"]) {
		    $result = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','0','$currenttime','$myrowname[UserName] has cleared their status','".time()."')",$db);
		}
		$result = mysql_query("UPDATE userlist SET Status='$changedstatus[1]' WHERE UserID=$addchatid[0]",$db);
		// FORCES A RELOAD ON THE USERLIST AND GAMELIST (BECAUSE WHEN YOU START A GAME, THEY ONLY LEAVE THE ROOM)
    	$result = mysql_query("UPDATE userlist SET PlayerListUpdate=0, GameListUpdate=0",$db);
    } elseif (substr(urldecode($command[2]),0,3)=="/me") {
		// WRITE A /ME STYLE MESSAGE
		////////////////////////////
		$mecommand = split ("me%20",$command[2],2);
		$resultname = mysql_query("SELECT * FROM userlist WHERE UserID=$addchatid[0]",$db);
		$myrowname = mysql_fetch_array($resultname);
		if ($mecommand[1]) {
			$result = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','0','$currenttime','$myrowname[UserName] $mecommand[1]','".time()."')",$db);
		}
	} elseif (substr(urldecode($command[2]),0,6)=="/clear") {
		// CLEAR THE SCREEN
		///////////////////
		$ic=0;
		$result = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','$addchatid[0]','','\nCHAT\n\nCHAT\n\nCHAT\n\nCHAT\n\nCHAT\n\nCHAT\n\nCHAT\n\nCHAT\n\nCHAT\n\nCHAT\n\nCHAT\n\nCHAT\n\n','".time()."')",$db);
		sleep(1);
		$result = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','$addchatid[0]','','*Screen cleared*','".time()."')",$db);
	} elseif (substr(urldecode($command[2]),0,16)=="/phra-totalranks") {
		// VIEW TOP 5 PHRA TOTAL RANKINGS
		/////////////////////////////////
		//include "command-phratotalranks.php";

	} elseif (substr(urldecode($command[2]),0,1)=="/") {
        // TELL THEM THIS COMMAND DOESN'T EXIST
        ///////////////////////////////////////
	    $result = mysql_query("INSERT INTO chat (UserID,ToUser,UserName,Message,TimeStamp) VALUES ('0','$addchatid[0]','$currenttime\IMR','This command does not exist.','".time()."')",$db);
	} elseif ($command[2]) {
 		// INSERT THE CHAT TO THE IMR
 		/////////////////////////////
    	$result = mysql_query("INSERT INTO chat (UserID,UserName,Message,TimeStamp) VALUES ('$command[1]','$currenttime$UserName','". addslashes($command[2])."','".time()."')",$db);
	}
	print "SUCCESS\n";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ADDING A GAME TO THE DATABASE 																			 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

if ($command[0]=="=ADD_GAME") {
	// FIND A SUITABLE SLOT OUT OF THE AVAILABLE GAME SPACES AVAILABLE TO ADD THE GAME INTO
    do {
        $gamecount++;
        $result = mysql_query("SELECT * FROM gamelist WHERE GameID=$gamecount",$db);
    	if (($myrow = mysql_fetch_array($result))==FALSE) {
    	    // IF THERE IS NO GAME IN THE $GAMECOUNT SLOT, IT WILL PUT THE CLIENTS GAME HERE
    	    // STRIP - OFF OF THE USERID AND UPDATE THE TIMESTAMP
    		$addgameid = split("-",$command[1]);
            $resultmts = mysql_query("SELECT * FROM userlist WHERE UserID=$addgameid[0]",$db);
			$myrowmts = mysql_fetch_array($resultmts);
			$ip = $myrowmts["IPAddress"];
			if ($ip==FALSE) {
			    $ip = "www.hoverrace.com/imr/randomgame.php";
			}
    	    $resultgame = mysql_query("INSERT INTO gamelist (GameID,UserID,GameName,Track,Laps,Weapons,GameIP,Port) VALUES ('$gamecount','$command[1]','" . addslashes($command[2]) ."','" . addslashes($command[3]) ."','$command[4]','$command[5]','$ip','$command[6]')",$db);
    	    $resultgameleave = mysql_query("INSERT INTO gameleave (GameID,HostName,GameName,Laps,Weapons) VALUES ('$gamecount','$myrowmts[UserName]','" . addslashes($command[2]) ."','$command[4]','$command[5]')",$db);
    	    $resultgameplayers = mysql_query("INSERT INTO gameplayers (GameID,UserID,JoinOrder) VALUES ('$gamecount','$command[1]','1')",$db);
    	    // THIS TELLS THE GAMELIST TO UPDATE ON REFRESH
    	    $result = mysql_query("UPDATE userlist SET StartedGame=".time()." WHERE UserID='$command[1]'",$db);
			print "SUCCESS\n";
			print "GAME_ID $gamecount-$command[1]\n";
			$gotgameslot=TRUE;
    	} elseif ($gamecount==$maxgames) {
    	    // THIS WARNS THAT THERE ARE ALREADY ALL GAME SLOTS FILLED
    	    print "ERROR 402\n" ;
    	    exit;
    	}
    } while ($gotgameslot==FALSE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CHECK A GAMES CONNECTIVITY TO ENSURE IT WORKS															 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

if ($command[0]=="=CHECK_GAME") {
    if($command[4]!=-1)
	{
		$resultmts = mysql_query("SELECT * FROM gamelist WHERE GameID=$command[4] AND UserID=$command[1]",$db);
		$myrowmts = mysql_fetch_array($resultmts);
		$gamename = $myrowmts["GameName"];
	}
	if(@fsockopen($command[2], $command[3], $errno, $errstr, '4'))
	{
		if($command[4]!=-1) $result = mysql_query("UPDATE gamelist SET GameName='" . urlencode("+") . " " . addslashes($gamename) . "' WHERE GameID=$command[4] AND UserID=$command[1]",$db);
	}
	else
	{
	    $resultmts = mysql_query("SELECT * FROM userlist WHERE UserID=$command[1]",$db);
		$myrowmts = mysql_fetch_array($resultmts);
	    if($command[4]!=-1) $result = mysql_query("INSERT INTO chat (UserID,UserName,Message,TimeStamp) VALUES ('$command[1]','$currenttime\IMR','$myrowmts[UserName]\'s game might have connection issues!',". time() .")",$db);
	    $result = mysql_query("INSERT INTO chat (UserID,UserName,ToUser,Message,TimeStamp) VALUES ('$command[1]','$currenttime\IMR', $command[1],'You appear to be behind a router or firewall:  Visit: http://www.hoverrace.com/?page=unabletoconnect',". time() .")",$db);
	    
	    $result = mysql_query("UPDATE gamelist SET GameName='" . urlencode("-") . " " . addslashes($gamename) . "' WHERE GameID=$command[4] AND UserID=$command[1]",$db);
	}
	$result = mysql_query("UPDATE userlist SET PlayerListUpdate=0, GameListUpdate=0",$db);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// REMOVING A GAME FROM THE DATABASE 																		 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

if ($command[0]=="=DEL_GAME") {
	// DELETES THE GAME FROM THE GAMELIST
    $result = mysql_query("DELETE FROM gamelist WHERE GameID='$command[1]' and UserID='$command[2]' LIMIT 1",$db);
    $result = mysql_query("DELETE FROM gameleave WHERE GameID='$command[1]' LIMIT 1",$db);
    $result = mysql_query("DELETE FROM gameplayers WHERE GameID='$command[1]'",$db);
    // TELLS THE CLIENT TO UPDATE HIS GAMELIST
    $result = mysql_query("UPDATE userlist SET StartedGame=".time()." WHERE UserID='$command[2]'",$db);
    print "SUCCESS\n";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// JOINING A GAME 																							 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

if ($command[0]=="=JOIN_GAME") {
	// ASSIGN A PLAYER A SLOT IN THE GAME FROM 2-10 (JOINORDER IS SIMPLY FOR DISPLAY PURPOSES ON THE PLAYERLIST)
	for ($playersingame=2; $playersingame<=$maxplayersingame; $playersingame++) {
	$resultmts = mysql_query("SELECT * FROM gameplayers WHERE JoinOrder=$playersingame",$db);
		if (($myrow = mysql_fetch_array($resultmts))==FALSE) {
		    // INSERT INTO THE GAMEPLAYERS DATABASE (FOR DISPLAYING ON THE PLAYERLIST)
    		$resultgameplayers = mysql_query("INSERT INTO gameplayers (GameID,UserID,JoinOrder) VALUES ('$command[1]','$command[2]','$playersingame')",$db);
    		// INSERT PLAYERS NAME INTO GAMELEAVE TABLE
    		// FIRST GET THE INFO ALREADY IN THE PLAYERNAMES COLUMN
    		$resultgameleaveadd = mysql_query("SELECT * FROM gameleave WHERE GameID='$command[1]'",$db);
    		$myrowleave = mysql_fetch_array($resultgameleaveadd);
    		// THEN GET THE PLAYERS NAME
    		$resultgameleavename = mysql_query("SELECT * FROM userlist WHERE UserID='$command[2]'",$db);
    		$myrowname = mysql_fetch_array($resultgameleavename);
    		// CREATE A NEW STRING FOR THE PLAYERSNAME COLUMN BASED ON WHO JUST JOINED!
			$newpn = $myrowleave["PlayerNames"] . "[$playersingame]$myrowname[UserName]<$playersingame>";
    		$resultgameplayers = mysql_query("UPDATE gameleave SET PlayerNames='$newpn' WHERE GameID='$command[1]'",$db);
    		// CUT SHORT THE FOR LOOP
    		$playersingame=$maxplayersingame+1;
    		// TELLS THE CLIENT TO UPDATE HIS GAMELIST
    		$result = mysql_query("UPDATE userlist SET StartedGame=".time()." WHERE UserID='$command[2]'",$db);
    		// CHECK THE PLAYER CAN CONNECT
    		if($command[3] and $command[4]) file_get_contents("http://" . $_SERVER["SERVER_ADDR"] . $_SERVER['PHP_SELF'] . "?=CHECK_GAME%%$command[2]%%$playerip%%$command[3]%%-1");
		}
	}
	print "SUCCESS\n";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LEAVING A GAME 																							 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

if ($command[0]=="=LEAVE_GAME") {
	// REMOVE THEMSELVES FROM THE GAMEPLAYERS DATABASE(AND THE GAMES PLAYERLIST)
    $result = mysql_query("DELETE FROM gameplayers WHERE GameID='$command[1]' and UserID='$command[2]'",$db);
    // TELLS THE CLIENT TO UPDATE HIS GAMELIST
    $result = mysql_query("UPDATE userlist SET StartedGame=".time()." WHERE UserID='$command[2]'",$db);
    
	RefreshPlayerNames($command[1]);
        
    print "SUCCESS\n";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DISPLAYING THE PEOPLE IN THE IMR 																							 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

if ($command[0]=="=WWW_ULIST") {
	// CALLS THE USERLIST
    $result = mysql_query("SELECT * FROM userlist ORDER BY UserID",$db);
    $myrow = mysql_fetch_array($result);
    // FAKE THE OLD LOOK OF THE USERLIST FOR THE SAKE OF THE IMR DISPLAY ON HR.COM
    print "<HTML>";
	print "<meta http-equiv=\"Refresh\" Content=8 >\n";
	print "<BODY BGCOLOR=\"FFFFFF\" TEXT=\"000000\">\n";
	print "<table border=\"2\" bgcolor=\"E0DFE3\"><tr><td align=\"center\" width=\"150\">Users List</td></tr></table>\n";
	print "<pre><FONT FACE=\"MS Sans Serif\" STYLE=\"font-size : 8 pt\">\n";
	// PRINT EACH NAME
	do {
	print urldecode($myrow[UserName]) . "\n";
	} while ($myrow = mysql_fetch_array($result));
	// END OF FAKING
	print "</font></pre>\n";
	print "</BODY></HTML>";
}
?>
