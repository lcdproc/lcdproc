#!/usr/bin/python3

import sys
import configparser
import subprocess
import argparse
import os
import collections


def array_index(index):
    return "#" + "_" * (len(str(index)) - 1) + str(index)


def read_lcdd(file):
    class ConfigParserMultiValues(collections.OrderedDict):
        def __setitem__(self, key, value):
            if key in self and isinstance(value, list):
                self[key].extend(value)
            else:
                super().__setitem__(key, value)

        @staticmethod
        def getlist(value):
            return value.split(os.linesep)

    config = configparser.RawConfigParser(strict=False, empty_lines_in_values=False, dict_type=ConfigParserMultiValues, converters={
                                          "list": ConfigParserMultiValues.getlist})
    config.read_file(file)

    kdb_config = {}
    for section in config.sections():
        if section == "server":
            section_key = "server/"
        elif section == "menu":
            section_key = "menu/"
        else:
            section_key = "driver/" + section.lower() + "/"

        for key in config[section]:
            kdb_key = section_key + key.lower()

            if (section == "server" and key.lower() in ["hello", "goodbye"]) or \
                (section.lower() == "hd44780" and key.lower() == "backlight") or \
                    (section.lower() == "linux_input" and key.lower() == "key"):
                kdb_config[kdb_key] = config[section].getlist(key)
            elif section == "server" and key.lower() == "driver":
                kdb_config[kdb_key] = ["driver/" + d for d in config[section].getlist(key)]
            else:
                kdb_config[kdb_key] = config[section][key]

    return kdb_config


def read_lcdproc(file):
    config = configparser.RawConfigParser()
    config.read_file(file)

    kdb_config = {}
    for section in config.sections():
        for key in config[section]:
            kdb_key = section.lower() + "/" + key.lower()
            kdb_config[kdb_key] = config[section][key]

    return kdb_config


class LcdexecReader():
    def __init__(self, config):
        self.kdb_config = {}
        self.config = config
        self.menu_index = 1
        self.command_index = 0
        self.parameter_index = {
            "slider": 0,
            "checkbox": 0,
            "numeric": 0,
            "ring": 0,
            "alpha": 0,
            "ip": 0
        }

    def read_param(self, param, param_key):
        for key in self.config[param]:
            self.kdb_config[param_key + key.lower()] = self.config[param][key]

    def read_command(self, section, command_key):
        if "DisplayName" in self.config[section]:
            self.kdb_config[command_key + "displayname"] = self.config[section]["DisplayName"]

        self.kdb_config[command_key + "exec"] = self.config[section]["Exec"]
        if "Feedback" in self.config[section]:
            self.kdb_config[command_key + "feedback"] = self.config[section]["Feedback"]
        if "Parameter" in self.config[section]:
            params = self.config.getlist(section, "Parameter")

            self.kdb_config[command_key + "parameters"] = []
            for param in params:
                param_type = self.config[param]["Type"].lower()
                param_key = "menu/parameter/"
                param_key += param_type + "/" + array_index(self.parameter_index[param_type])
                self.parameter_index[param_type] += 1
                self.kdb_config[command_key + "parameters"].append(param_key)
                self.read_param(param, param_key + "/")

    def read_menu_entries(self, section, menu_key):
        entries = self.config.getlist(section, "Entry")
        self.kdb_config[menu_key + "entries"] = []
        for entry in entries:
            if "Exec" in self.config[entry]:
                entry_key = "menu/command/" + array_index(self.command_index)
                self.command_index += 1
                self.read_command(entry, entry_key + "/")
            else:
                entry_key = "menu/menu/" + array_index(self.menu_index)
                self.menu_index += 1
                self.read_menu(entry, entry_key + "/")
            self.kdb_config["menu/menu/#0/entries"].append(entry_key)

    def read_menu(self, section, menu_key):
        if "DisplayName" in self.config[section]:
            self.kdb_config[menu_key + "displayname"] = self.config[section]["DisplayName"]

        self.read_menu_entries(section, menu_key)

    def read_main_menu(self):
        self.kdb_config["menu/main"] = "menu/menu/#0"
        self.read_menu_entries("MainMenu", "menu/menu/#0/")
        self.kdb_config["menu/menu"] = [""] * self.menu_index
        self.kdb_config["menu/command"] = [""] * self.command_index
        for param_type, param_index in self.parameter_index.items():
            self.kdb_config["menu/parameter/" + param_type] = [""] * param_index
        return self.kdb_config


def read_lcdexec(file):
    class ConfigParserMultiValues(collections.OrderedDict):

        def __setitem__(self, key, value):
            if key in self and isinstance(value, list):
                self[key].extend(value)
            else:
                super().__setitem__(key, value)

        @staticmethod
        def getlist(value):
            return value.split(os.linesep)

    config = configparser.ConfigParser(strict=False, empty_lines_in_values=False, dict_type=ConfigParserMultiValues,
                                       converters={"list": ConfigParserMultiValues.getlist})
    config.read_file(file)

    kdb_config = {}
    if "lcdexec" in config.sections():
        for key in config["lcdexec"]:
            kdb_key = "lcdexec/" + key.lower()
            kdb_config[kdb_key] = config["lcdexec"][key]

    if "MainMenu" in config.sections():
        kdb_config.update(LcdexecReader(config).read_main_menu())

    return kdb_config


def read_lcdvc(file):
    config = configparser.RawConfigParser()
    config.read_file(file)

    kdb_config = {}
    for section in config.sections():
        for key in config[section]:
            kdb_key = "lcdvc/" + key.lower()
            kdb_config[kdb_key] = config[section][key]

    return kdb_config


def check_kdb_tmp():
    cmd = ["kdb", "ls", "user/sw/lcdproc/tmp"]
    proc = subprocess.run(cmd, stdout=subprocess.PIPE)
    if len(proc.stdout.decode("utf-8")) > 0:
        print("Please ensure that user/sw/lcdproc/tmp is empty", file=sys.stderr)
        exit(1)


def write_to_kdb(kdb_config):
    def write_value(key, value):
        if value.startswith('"') and value.endswith('"'):
            value = value[1:-1]
        cmd = ["kdb", "set", "--", "user/sw/lcdproc/tmp/" + str(key), str(value)]
        subprocess.run(cmd, stdout=subprocess.DEVNULL)

    def write_meta(key, meta, value):
        if value.startswith('"') and value.endswith('"'):
            value = value[1:-1]
        cmd = ["kdb", "setmeta", "--", "user/sw/lcdproc/tmp/" + str(key), str(meta), str(value)]
        subprocess.run(cmd, stdout=subprocess.DEVNULL)

    for key, value in kdb_config.items():
        if isinstance(value, list):
            write_meta(key, "array", array_index(len(value) - 1))
            for index, item in enumerate(value):
                write_value(str(key) + "/" + array_index(index), item)
        elif value.lower() in ["false", "n", "no", "off", "true", "y", "yes", "on"]:
            write_value(key, value.lower())
        else:
            write_value(key, value)


def export_kdb_tmp(outfile):
    cmd = ["kdb", "export", "user/sw/lcdproc/tmp", "yamlcpp", outfile.name]
    subprocess.run(cmd, stdout=subprocess.DEVNULL)


def clean_kdb_tmp():
    cmd = ["kdb", "rm", "-r", "user/sw/lcdproc/tmp"]
    subprocess.run(cmd, stdout=subprocess.DEVNULL)


parser = argparse.ArgumentParser()
parser.add_argument("mode")
parser.add_argument("infile", type=argparse.FileType('r', encoding="UTF-8"))
parser.add_argument("outfile", type=argparse.FileType('w', encoding="UTF-8"))

args = parser.parse_args()
mode = args.mode.lower()

check_kdb_tmp()

if mode == "lcdd":
    print("Not supported right now", file=sys.stderr)
    exit(1)
    kdb_config = read_lcdd(args.infile)
elif mode == "lcdproc":
    kdb_config = read_lcdproc(args.infile)
elif mode == "lcdvc":
    kdb_config = read_lcdvc(args.infile)
elif mode == "lcdexec":
    kdb_config = read_lcdexec(args.infile)
else:
    print("unknown mode", file=sys.stderr)
    exit(1)

write_to_kdb(kdb_config)

export_kdb_tmp(args.outfile)
clean_kdb_tmp()

print("Success: Please mount '" + args.outfile.name + "' with yamlcpp under 'user/sw/lcdproc/" + mode + "/#0/current'")
