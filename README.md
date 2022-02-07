# Fast64
[![Scorecards supply-chain security](https://github.com/bancolombia/fast64_elixir/actions/workflows/scorecards-analysis.yml/badge.svg)](https://github.com/bancolombia/fast64_elixir/actions/workflows/scorecards-analysis.yml)

High performance Elixir base64 encoder/decoder in C
20x faster than Elixir default implementation.

## Installation

If [available in Hex](https://hex.pm/docs/publish), the package can be installed
by adding `fast64` to your list of dependencies in `mix.exs`:

```elixir
def deps do
  [
    {:fast64, "~> 0.1.3"}
  ]
end

#How to

def sample do
  encoded = Fast64.encode64("hello")
  decoded = Fast64.decode64("aGVsbG8=")
end

```

Documentation can be generated with [ExDoc](https://github.com/elixir-lang/ex_doc)
and published on [HexDocs](https://hexdocs.pm). Once published, the docs can
be found at [https://hexdocs.pm/fast64](https://hexdocs.pm/fast64).

