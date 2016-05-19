Keylogger as Linux kernel module
--------------------------------

###Installation & Configuration

#####Client side:

Go to ```client``` directory and run below commands.
```
$ make
$ ./install.sh
$ make clean
```

#####Server side:

Go to ```server``` directory and run below command.
```
$ python server.py
```

Client ID in ```config.ini``` is the first free id which can be assign to client. Before first
start of the server make sure that it is set to 0.


###Requirements

Requires Python 3.x version.

###Tests

Unfortunately client side (keylogger) is not testable. Server side tests are located in
```/server/tests/``` directory.

```parser_test.py``` - checks whether logged keys are properly  transformed to readable format

```protocol_test.py``` - checks whether server properly responses for given set of requests and messages

###Git Access

The source repository is available from the GitHub:

  https://github.com/mkpaszkiewicz/visual-search-engine.git<br>
  git@github.com:mkpaszkiewicz/visual-search-engine.git

### Authors
Konrad Sikorski<br>
Marcin K. Paszkiewicz <mkpaszkiewicz@gmail.com><br>
*Warsaw University of Technology*