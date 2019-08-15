- refactor chatbox code
- fix that nullptr issue
- connect to database



- make it load 50 messages at first

- then make it load 50 more messages when at top of that, and 50 more and so on

- keep the jump to bottom thing for now

  - but you'll need to make a special case of it to prevent jump to bottom when old messages are loaded

  - run a checker on another thread to check for old messages every 5 seconds and deletes anything beyond last 50 messages

    - it should loop through all of the elements, and delete all but newest 50

    - and add

      - the database for messages should be like these columns: 

      + msgID (primary key), roomNetworkID (so in form [roomID.networkID]), relativeMsgID (so auto increment ID of the msg relative to that specific room)

      - so with this, there may be 100 msgs in room.network, and a msgID could be 4352 but relativeMsgID can be 100, so we can discern exactly what the newest ones are
      - realistically separate roomNetworkID into two numbers, and then store roomID, networkID, msgID and relativeMsg as 4 integers in the msgContainer panel, and I've edited the include files so that the 4 integers are public variables of tgui::Panel



- fix the button scaling, so like 0.7 for everything
- get rid of loading animation, too slow, just have "Loading..." instead
- make the buttons redirect to empty pages wherever necessary (like in the designs), and make the tabs in those too