#!env python3

import sys
import configparser
from typing import Dict, IO
from subprocess import run
import argparse
import os
import collections

def read_lcdd(file: IO):
    class ConfigParserMultiValues(collections.OrderedDict):
        def __setitem__(self, key, value):
            if key in self and isinstance(value, list):
                self[key].extend(value)
            else:
                super().__setitem__(key, value)

        @staticmethod
        def getlist(value):
            return value.split(os.linesep)

    config = configparser.RawConfigParser(strict=False, empty_lines_in_values=False, dict_type=ConfigParserMultiValues, converters={"list": ConfigParserMultiValues.getlist})
    config.read_file(file)

    base_key = "/sw/lcdproc/lcdd/#0/current/"

    kdb_config = {}
    for section in config.sections():
        if section == "server":
            section_key = "server/"
        elif section == "menu":
            section_key = "menu/"
        else:
            section_key = "driver/" + section.lower() + "/"

        for key in config[section]:
            kdb_key = base_key + section_key + key.lower()

            if (section == "server" and key.lower() in ["hello", "goodbye"]) or \
                (section.lower() == "hd44780" and key.lower() == "backlight") or \
                (section.lower() == "linux_input" and key.lower() == "key"):
                kdb_config[kdb_key] = config[section].getlist(key)
            elif section == "server" and key.lower() == "driver":
                kdb_config[kdb_key] = [base_key+"driver/"+d for d in config[section].getlist(key)]
            else: 
                kdb_config[kdb_key] = config[section][key]

    return kdb_config


def read_lcdproc(file: IO):
    config = configparser.RawConfigParser()
    config.read_file(file)

    base_key = "/sw/lcdproc/lcdproc/#0/current/"

    kdb_config = {}
    for section in config.sections():
        if section == "lcdproc":
            section_key = "general/"
        else:
            section_key = "screenmode/" + section.lower() + "/"

        for key in config[section]:
            kdb_key = base_key + section_key + key.lower()
            kdb_config[kdb_key] = config[section][key]

    return kdb_config


def read_lcdexec(file: IO):
    class ConfigParserMultiValues(collections.OrderedDict):

        def __setitem__(self, key, value):
            if key in self and isinstance(value, list):
                self[key].extend(value)
            else:
                super().__setitem__(key, value)

        @staticmethod
        def getlist(value):
            return value.split(os.linesep)

    config = configparser.ConfigParser(strict=False, empty_lines_in_values=False, dict_type=ConfigParserMultiValues, converters={
                                       "list": ConfigParserMultiValues.getlist})
    config.read_file(file)

    base_key = "/sw/lcdproc/lcdexec/#0/current/"

    kdb_config = {}
    if "lcdexec" in config.sections():
        for key in config["lcdexec"]:
            kdb_key = base_key + "general/" + key.lower()
            kdb_config[kdb_key] = config["lcdexec"][key]

    def read_param(param: str, param_key: str):
        for key in config[param]:
            kdb_config[param_key+key.lower()] = config[param][key]

    def read_command(section: str, command_key: str):
        if "DisplayName" in config[section]:
            kdb_config[command_key+"displayname"] = config[section]["DisplayName"]

        kdb_config[command_key+"exec"] = config[section]["Exec"]
        if "Feedback" in config[section]:
            kdb_config[command_key+"feedback"] = config[section]["Feedback"]
        if "Parameter" in config[section]:
            params = config.getlist(section, "Parameter")

            kdb_config[command_key+"param"] = "#"+str(len(params)-1)
            for index, param in enumerate(params):
                param_key = base_key+"menu/param/"+config[param]["Type"].lower()+"/"+param.lower()
                kdb_config[command_key+"param/#"+str(index)] = param_key
                read_param(param, param_key+"/")

    def read_menu_entries(section: str, menu_key: str):
        entries = config.getlist(section, "Entry")
        kdb_config[menu_key+"entry"] = "#"+str(len(entries)-1)
        for index, entry in enumerate(entries):
            if "Exec" in config[entry]:
                entry_key = base_key+"menu/command/"+entry.lower()
                read_command(entry, entry_key+"/")
            else:
                entry_key = base_key+"menu/menu/"+entry.lower()
                read_menu(entry, entry_key+"/")
            kdb_config[base_key+"menu/menu/main/entry/#"+str(index)] = entry_key

    def read_menu(section: str, menu_key: str):
        if "DisplayName" in config[section]:
            kdb_config[menu_key+"displayname"] = config[section]["DisplayName"]
        
        read_menu_entries(section, menu_key)

    if "MainMenu" in config.sections():
        kdb_config[base_key+"menu/main"] = base_key+"menu/menu/main"
        read_menu_entries("MainMenu", base_key+"menu/menu/main/")

    return kdb_config


def read_lcdvc(file: IO):
    config = configparser.RawConfigParser()
    config.read_file(file)

    base_key = "/sw/lcdproc/lcdvc/#0/current/"

    kdb_config = {}
    for section in config.sections():
        for key in config[section]:
            kdb_key = base_key + key.lower()
            kdb_config[kdb_key] = config[section][key]

    return kdb_config


def write_to_kdb(kdb_config):
    def write_value(key, value):
        if value.startswith('"') and value.endswith('"'):
            value = value[1:-1]
        cmd = ["kdb", "set", "--", f"{key}", f"{value}"]
        print("$", " ".join(cmd))
        run(cmd)
    
    for key, value in kdb_config.items():
        if isinstance(value, list):
            write_value(key, f"#{len(value) - 1}")
            for index,item in enumerate(value):
                write_value(f"{key}/#{index}", item)
        else:
            write_value(key, value)


parser = argparse.ArgumentParser()
parser.add_argument("mode")
parser.add_argument("file", type=argparse.FileType('r', encoding="UTF-8"))

args = parser.parse_args()
mode = args.mode.lower()

if mode == "lcdd":
    kdb_config = read_lcdd(args.file)
elif mode == "lcdproc":
    kdb_config = read_lcdproc(args.file)
elif mode == "lcdvc":
    kdb_config = read_lcdvc(args.file)
elif mode == "lcdexec":
    kdb_config = read_lcdexec(args.file)
else:
    print("unknown mode")
    exit(1)

write_to_kdb(kdb_config)
