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
      app: :fast_64,
      version: "0.1.0",
      elixir: "~> 1.10",
      start_permanent: Mix.env() == :prod,
      compilers: [:fast64] ++ Mix.compilers(),
      deps: deps()
    ]
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
      # {:dep_from_hexpm, "~> 0.3.0"},
      {:ex_doc, ">= 0.0.0", only: :dev, runtime: false}
      # {:dep_from_git, git: "https://github.com/elixir-lang/my_dep.git", tag: "0.1.0"}
    ]
  end

  defp package do
    [
      maintainers: ["Daniel Bustamante Ospina"],
      licenses: ["Apache-2.0"],
      links: %{"GitHub" => "https://github.com/dbuos/tellurium"}
    ]
  end

end
