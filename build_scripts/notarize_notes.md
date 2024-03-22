# How to notarize the apple installers

1. Create an [app specific password](https://support.apple.com/en-gb/HT204397) with name `altool` in your Apple ID account. Before clicking done, make sure you take a note of this password, it will usually be of the form `abcd-efgh-ijkl-mnop`.

2. At the command line, run 

        xcrun altool --store-password-in-keychain-item "AC_PASSWORD" -u michael.berks@manchester.ac.uk -p <password>

   Where `<password>` is the app specific password you created in step 1. Substitute your own email for your Apple ID account. This associates the app specific password with your Apple ID, so future commands just specify the keychain entry now stored `AC_PASSWORD`, populating both the username and password fields of the command.

3. Submit the DMG installer image for notarization by running

        xcrun altool --notarize-app --primary-bundle-id "com.qbi-lab.madym.mac11.gui.dmg" -f "madym_v4.16.1-Darwin-20.1.0.dmg" -p "@keychain:AC_PASSWORD"

    The `primary-bundle-id` can be set to anything to identify your submission, but can only use alphanumeric characters (A-Z, a-z, 0-9), hyphen (-), and period (.). The notarization service includes the value whenever it emails you regarding the given `altool` submission.

4. Make a note of the request ID, this will also be in the email you receive from Apple. In the example below, the ID is `589ba030-485b-43a8-b65d-13ab3f80c19c`

5. If the notarization failed, check the log by first running

        xcrun altool --notarization-info 589ba030-485b-43a8-b65d-13ab3f80c19c -p "@keychain:AC_PASSWORD"

    This will return a results like

        No errors getting notarization info.

        Date: 2022-01-11 11:41:55 +0000
        Hash: 528a1ec899f0afc43af4a7c66f4c51dd390b12b3332beb96ade85e942c240b48
        LogFileURL: https://osxapps-ssl.itunes.apple.com/itunes-assets/Enigma116/v4/1c/44/f6/1c44f6f6-d228-b2c9-b36d-09eaec954757/developer_log.json?accessKey=1642095855_725428822842527928_fViqiyegc23xz%2BSR0SeOMVP0buEipbQ4IbVyS0dw7V5ATlBr6tSIsb8ELrBP7TpRDN1%2BRDReHKn4hfAk8K825PGqaFjywClxATZySUt9SUPyjJLcX2HjWnWcIE0EOIw7yZ8lA0Qs4NwKsWhnWq%2BrepYf1vR%2B1EfMB68waPqzTLU%3D
        RequestUUID: 589ba030-485b-43a8-b65d-13ab3f80c19c
        Status: invalid
        Status Code: 2
        Status Message: Package Invalid

    The log URL can then be copied into a browser to inspect what went wrong

6. If all succeeded, complete the process by stapling the notarized certificate to the installer by running

        xcrun stapler staple "madym_v4.16.1-Darwin-20.1.0.dmg"