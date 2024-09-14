import os, shutil, winreg, argparse, sys

def file_cmd(args):
    if args.name == None: sys.exit('Incorrect input')

    if args.cmd == 'create': 
        f = open(args.name, "w")
        f.close()
    elif args.cmd == 'remove':
        os.remove(args.name)
    elif args.cmd == 'read':
        f = open(args.name, 'r')
        print(f.read())
        f.close()
    elif args.cmd == 'write':
        if args.value == None: sys.exit('Icorrect input')
        f = open(args.name, 'a')
        f.write(args.value)
        f.close()
    elif args.cmd == 'copy':
        if args.dst == None: sys.exit('Icorrect input')
        shutil.copyfile(args.name, args.dst)
    elif args.cmd == 'rename':
        if args.dst == None: sys.exit('Icorrect input')
        os.rename(args.name, args.dst)
    else:
        sys.exit('Icorrect input')

def reg_cmd(args):
    if args.key == None: sys.exit('Incorrect input')

    location = winreg.HKEY_CURRENT_USER
    if args.cmd == 'create':
        k = winreg.CreateKey(location, args.key)
        if k: winreg.CloseKey(k)
    elif args.cmd == 'remove':
        k = winreg.OpenKeyEx(location, args.key)
        winreg.DeleteKey(k, "")
        if k: winreg.CloseKey(k)
    elif args.cmd == 'write':
        if args.value == None: sys.exit('Incorrect input')
        k = winreg.OpenKeyEx(location, args.key)
        winreg.SetValueEx(k, args.key, 0, winreg.REG_SZ, args.value)
        if k: winreg.CloseKey(k)       
    else:
        sys.exit('Icorrect input')

def helper(args):
    if args.type == 'file':
        file_cmd(args)
    elif args.type == 'reg':
        reg_cmd(args)
    else:
        sys.exit('Incorrect type')

def main():
    parser = argparse.ArgumentParser(
        prog='FS_util',
    )

    parser.add_argument('type')             # file/reg
    parser.add_argument('cmd')              # create/remove/read/write/copy/rename
    parser.add_argument('-n', '--name')     # filename
    parser.add_argument('-d', '--dst')      # destination
    parser.add_argument('-k', '--key')      # key
    parser.add_argument('-v', '--value')    # value

    args = parser.parse_args()
    helper(args)

if __name__ == "__main__":
    main()