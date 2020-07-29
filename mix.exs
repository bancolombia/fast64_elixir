defmodule Mix.Tasks.Compile.Fast64 do

  def run(_) do
    {result, _error_code} = System.cmd("make", ["priv/fast64.so"], stderr_to_stdout: true)
    IO.binwrite result
    :ok
  end

  def clean() do
    System.cmd("rm", ["-f", "priv/fast64.so"])
  end

end

defmodule Fast64.MixProject do
  use Mix.Project

  def project do
    [
      app: :fast64,
      version: "0.1.3",
      elixir: "~> 1.10",
      start_permanent: Mix.env() == :prod,
      compilers: [:fast64] ++ Mix.compilers(),
      description: description(),
      package: package(),
      deps: deps()
    ]
  end

  defp description() do
    "High performance Elixir base 64 encoder/decoder in C."
  end

  # Run "mix help compile.app" to learn about applications.
  def application do
    [
      extra_applications: [:logger]
    ]
  end

  # Run "mix help deps" to learn about dependencies.
  defp deps do
    [
      {:ex_doc, ">= 0.0.0", only: :dev, runtime: false}
    ]
  end

  defp package do
    [
      maintainers: ["Daniel Bustamante Ospina"],
      licenses: ["Apache-2.0"],
      links: %{"GitHub" => "https://github.com/bancolombia/fast64_elixir"},
      files: ["lib", "priv/.gitignore", "mix.exs", "README.md",  "src", "Makefile"]
    ]
  end

end
