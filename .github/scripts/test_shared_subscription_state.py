#!/usr/bin/env python3
"""Check shared-subscription retries without a broker or client processes."""
import importlib.util
import io
from pathlib import Path
import unittest
from unittest.mock import patch


class ImmediateThread:
    def __init__(self, target, args):
        self.target, self.args = target, args

    def start(self):
        self.target(*self.args)


class SharedSubscriptionStateTest(unittest.TestCase):
    def load_script(self, name):
        spec = importlib.util.spec_from_file_location(
            name, Path(__file__).with_name(name + ".py"))
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        return module

    def run_attempt(self, module, messages):
        processes = []

        def popen(*args, **kwargs):
            process = type("Client", (), {})()
            process.stdout = io.StringIO("message\n" * messages[len(processes)])
            processes.append(process)
            return process

        with patch.object(module.subprocess, "Popen", side_effect=popen), \
             patch.object(module.threading, "Thread", ImmediateThread), \
             patch.object(module.time, "sleep"), \
             patch.object(module, "stop_process") as stop, \
             patch("builtins.print"):
            result = module.test_shared_subscription()
            self.assertEqual(len(processes), 6)
            self.assertTrue(all(any(call.args[0] is process
                                    for call in stop.call_args_list)
                                for process in processes))
            return result

    def test_repeat_success(self):
        for name in ("mqtt_test_v5", "tls_v5_test"):
            with self.subTest(script=name):
                module = self.load_script(name)
                for _ in range(2):
                    self.assertTrue(self.run_attempt(module, [4, 3, 3, 10, 10, 0]))

    def test_failure_then_success(self):
        for name in ("mqtt_test_v5", "tls_v5_test"):
            with self.subTest(script=name):
                module = self.load_script(name)
                self.assertFalse(self.run_attempt(module, [1, 0, 0, 0, 0, 0]))
                self.assertTrue(self.run_attempt(module, [4, 3, 3, 10, 10, 0]))

    def test_missing_messages_fail(self):
        for name in ("mqtt_test_v5", "tls_v5_test"):
            with self.subTest(script=name):
                module = self.load_script(name)
                self.assertFalse(self.run_attempt(module, [4, 3, 2, 10, 10, 0]))


if __name__ == "__main__":
    unittest.main()
