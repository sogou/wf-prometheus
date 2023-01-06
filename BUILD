load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary")

cc_library(
	name = "wf_prometheus",
	hdrs = glob(["**/*.h"]),
	srcs = glob(["src/*.cc"]),
	deps = [
		"@workflow//:http",
		"@workflow//:upstream",
	],
	linkopts = [
		"-lpthread",
		"-lssl",
		"-lcrypto",
	],
	visibility = ["//visibility:public"]
)

cc_library(
	name = "wf_prometheus_hdrs",
	hdrs = glob(['src/include/wf-prometheus/*']),
	includes = ['src/include'],
	visibility = ["//visibility:public"],
)

cc_binary(
	name = "example",
	srcs = ["example.cc"],
	copts = ["-Isrc/include/"],
	deps = [
		"//:wf_prometheus",
		"@workflow//:http",
		"@workflow//:upstream",
	],
)

