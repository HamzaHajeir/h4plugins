import os
import subprocess
import pandas as pd
import re
import shutil
import numpy as np

# Constants
DEFAULT_CONFIG = 'platformio.ini'
BUILD_DIR = 'build'
ASSISTANT_BUILD_DIR = 'extra-build-dir'
EXAMPLES_DIR = './examples'
COUNTER_START = 10000
CI_BRANCH_NAME = 'CI'
TESTS_DIR = 'tests/'

# Create necessary directories
os.makedirs(BUILD_DIR, exist_ok=True)

# Global array to store environments of the default configuration
DEFAULT_ENVIRONMENTS = []

from urllib.parse import quote

def convert_os_path_to_url(os_path):
    # Replace backslashes with forward slashes
    url_path = os_path.replace('\\', '/')

    # Encode the URL to handle any special characters/space
    url_path = quote(url_path, safe='/')

    return url_path


def gather_default_environments(config_file):
    """Populate the global array with environments from the default configuration."""
    print(f"Gathering default environments from {config_file}:")
    global DEFAULT_ENVIRONMENTS
    DEFAULT_ENVIRONMENTS = get_environments(config_file)
    print(DEFAULT_ENVIRONMENTS)

def create_environment_directories():
    # Create build directory if it does not exist
    os.makedirs(BUILD_DIR, exist_ok=True)

    for env in DEFAULT_ENVIRONMENTS:
        # Create env directory
        env_dir = os.path.join(BUILD_DIR, env)
        os.makedirs(env_dir, exist_ok=True)

        # Copy the default platformio.ini to the env directory
        shutil.copy(DEFAULT_CONFIG, os.path.join(env_dir, 'platformio.ini'))

        # Create src directory in the environment folder
        src_dir = os.path.join(env_dir, 'src')
        os.makedirs(src_dir, exist_ok=True)

        print(f"Created environment for {env} in {env_dir}")

    # Create extra directory
    assistant_build_dir = os.path.join(BUILD_DIR, ASSISTANT_BUILD_DIR)
    os.makedirs(assistant_build_dir, exist_ok=True)

    # Create src directory in the environment folder
    src_dir = os.path.join(assistant_build_dir, 'src')
    os.makedirs(src_dir, exist_ok=True)
    print(f"Created PIO environment for extra builders in {assistant_build_dir}")
    
def copy_example_to_env(example_path,env):
    # Define the src directory based on the environment
    build_dir = os.path.join(BUILD_DIR, env)
    src_dir = os.path.join(BUILD_DIR, env, 'src')

    # Remove previous example files in the src directory
    if os.path.exists(src_dir):
        shutil.rmtree(src_dir)  # Remove the existing src directory (and its contents)

    # Create the src directory again
    os.makedirs(src_dir)

    # Copy the example files to the src directory
    shutil.copytree(example_path, src_dir, dirs_exist_ok=True)  # Use dirs_exist_ok=True for Python 3.8+

    print(f"Copied example source files to {src_dir}")
    return build_dir

def copy_example_to_extra_build(example_path):
    # Define the src directory based on the environment
    build_dir = os.path.join(BUILD_DIR, ASSISTANT_BUILD_DIR)
    src_dir = os.path.join(BUILD_DIR, ASSISTANT_BUILD_DIR, 'src')

    # Remove previous example files in the src directory
    if os.path.exists(src_dir):
        shutil.rmtree(src_dir)  # Remove the existing src directory (and its contents)

    # Create the src directory again
    os.makedirs(src_dir)

    # Copy the example files to the src directory
    shutil.copytree(example_path, src_dir, dirs_exist_ok=True)  # Use dirs_exist_ok=True for Python 3.8+

    config_path = os.path.join(build_dir, 'platformio.ini')
    if os.path.exists(config_path):
        os.remove(config_path)
    shutil.copy(os.path.join(example_path,"platformio.ini"), config_path)

    print(f"Copied example files to {build_dir}")
    return build_dir


def find_example_paths():
    """Find all directories containing .ino files."""
    print("Finding example paths")
    example_paths = []
    for root, dirs, files in os.walk(EXAMPLES_DIR):
        if any(file.endswith('.ino') for file in files):
            example_paths.append(root)
    return example_paths

def trim_outer_directory(example_path):
    # Split the path into components
    parts = example_path.split(os.sep)

    # Check if there are at least two components to trim
    if len(parts) > 1:
        # Join the parts excluding the outer directory
        trimmed_path = os.sep.join(parts[1:])
        return trimmed_path
    else:
        # Return the original path if it doesn't have the structure
        return example_path

def get_configuration_file(example_path):
    """Identify the appropriate configuration file for a given example path."""
    config_file = os.path.join(example_path, DEFAULT_CONFIG)
    if os.path.isfile(config_file):
        return config_file
    return DEFAULT_CONFIG

def get_environments(config_file):
    """Retrieve available environments from the configuration file."""
    output = subprocess.run(f'pio run --project-conf {config_file} --environment all', shell=True, capture_output=True, text=True)
    valid_names_part = output.stderr.split("Valid names are ")
    envs = valid_names_part[1].strip("\n").strip("'").strip("`").split(", ")

    # envs = re.findall(r"Valid names are (.*)", output.stdout)
    return envs if envs else []


def build_example(example_name, build_path, environment):
    """Run the build command for the specified example and environment."""
    print(f"Building Example {example_name} environment {environment} within [{build_path}]:")
    command = f'pio run --environment {environment} --project-dir {build_path}'
    return subprocess.run(command, shell=True, capture_output=True)

def parse_memory_usage(log):
    """Parse memory usage from the build log."""
    ram_match = re.search(r'RAM:\s+\[.*?\]\s+(\d+\.\d+)%\s*\(used\s+(\d+)\s+bytes\s+from\s+(\d+)\s+bytes\)', str(log))
    flash_match = re.search(r'Flash:\s+\[.*?\]\s+(\d+\.\d+)%\s*\(used\s+(\d+)\s+bytes\s+from\s+(\d+)\s+bytes\)', str(log))

    ram_usage = ram_match.groups() if ram_match else (0, 0, 0)
    flash_usage = flash_match.groups() if flash_match else (0, 0, 0)
    
    return {
        'ram_percentage': ram_usage[0],
        'ram_used': int(ram_usage[1]),
        'ram_total': int(ram_usage[2]),
        'flash_percentage': flash_usage[0],
        'flash_used': int(flash_usage[1]),
        'flash_total': int(flash_usage[2])
    }

def create_report_table(results):
    """Create a DataFrame for the results and format it for Markdown output."""
    df = pd.DataFrame(results)

    print (df)

    # Combine rows based on 'Example'
    # We will use groupby and aggregate
    unified_df = df.groupby('Example').agg(lambda x: x.dropna().tolist()).reset_index()

    # Transform the lists back into single values, or keep them as lists
    for col in unified_df.columns[1:]:
        unified_df[col] = unified_df[col].apply(lambda x: x[0] if len(x) > 0 else np.nan)
    # Return the DataFrame

    return unified_df

def save_report(df, run_count):
    """Save the build results DataFrame as a Markdown file."""
    short_commit_hash = subprocess.run('git rev-parse --short HEAD', shell=True, capture_output=True, text=True).stdout.strip()
    report_name = f"{run_count}-{short_commit_hash}.md"
    report_path = os.path.join(TESTS_DIR, report_name)
    
    # Create directory if it doesn't exist
    os.makedirs(TESTS_DIR, exist_ok=True)
    
    # Save the report as a Markdown file
    with open(report_path, 'w') as f:
        f.write(df.to_markdown(index=False))
    return report_path

def switch_to_ci_branch():
    """Switch to the CI branch, creating it if it doesn't exist."""
    try:
        # Fetch the remote branches to ensure we have the latest state
        subprocess.run("git fetch origin", shell=True, check=True)
        
        # Checkout the CI branch, create if it doesn't exist
        subprocess.run(f"git checkout {CI_BRANCH_NAME}", shell=True, check=True)
    except subprocess.CalledProcessError:
        # Create and switch to the CI branch if it does not exist
        subprocess.run(f"git checkout -b {CI_BRANCH_NAME}", shell=True)

def push_report(run_count):
    """Push the Markdown report to the CI branch."""
    subprocess.run(f"git add {TESTS_DIR}", shell=True, check=True)
    subprocess.run(f"git add {os.path.join(BUILD_DIR, '**', '*.log')}", shell=True, check=True)
    
    commit_message = f"Add build report #{run_count}"
    subprocess.run(f"git commit -m \"{commit_message}\"", shell=True, check=True)
    
    try:
        subprocess.run(f"git push -u origin {CI_BRANCH_NAME}", shell=True, check=True)
    except subprocess.CalledProcessError:
        # Handle the case where the push fails and the local branch is out of sync
        print(f"Local branch {CI_BRANCH_NAME} is out of sync with remote. Pulling changes...")
        subprocess.run(f"git pull origin {CI_BRANCH_NAME}", shell=True, check=True)
        subprocess.run(f"git push -u origin {CI_BRANCH_NAME}", shell=True, check=True)

def create_pull_request(body_path, run_id):
    """Create a pull request to merge the CI branch into master."""
    # Read the content of the markdown file
    with open(body_path, 'r', encoding='utf-8') as file:
        markdown_content = file.read()
    subprocess.run(f"gh pr create --base master --head {CI_BRANCH_NAME} --title \"Build Report No.{run_id}\" --body \"{markdown_content}\"", shell=True)

def create_result_entry(example_path, env, build_result, log_file):
    """Create a result entry based on the build outcome, using environment as the column name."""
    base_entry = {
        'Example': trim_outer_directory(example_path)
    }
    line = ""
    if build_result.returncode == 0:
        memory_usage = parse_memory_usage(build_result.stdout)
        # Add RAM and Flash usage details to base_entry
        ram = f"RAM:{memory_usage['ram_used']} / {memory_usage['ram_total']} - {memory_usage['ram_percentage']}%<br>"
        flash = f"Flash:{memory_usage['flash_used']} / {memory_usage['flash_total']} - {memory_usage['flash_percentage']}%<br>"
        line = ':white_check_mark:<br>' + ram + flash + f"[Build Log]({log_file})"
    else:
        line = ':x:<br>' + f"[Build Log]({log_file})"

    # Use the environment as the column name
    if env in DEFAULT_ENVIRONMENTS:
        base_entry[env] = line
        return base_entry
    else:
        # Populate Shared-Build with the environment appended to the example name
        base_entry['Example'] = f"{base_entry['Example']} ({env})"
        base_entry['Shared-Build'] = line
        return base_entry

def format_log(log):
    # Check if log is in bytes
    if isinstance(log, bytes):
        log = log.decode('latin-1')  # Decode safely to avoid Unicode errors

    # First, replace `\r\n` with a single newline
    formatted_log = log.replace('\\r\\n', '\n')

    # Replace remaining `\n` and `\r` independently
    # Using regex to make sure to replace them only once
    formatted_log = re.sub(r'\\n(?![a-zA-Z])', '\n', formatted_log, count=1)
    formatted_log = re.sub(r'\\r(?![a-zA-Z])', '', formatted_log, count=1)

    # Remove any unintended blank lines resulting from replacements
    formatted_log = re.sub(r'\n\s*\n+', '\n', formatted_log).strip()  # Collapse multiple newlines

    return formatted_log


def main():
    pr_on_end = False
    if subprocess.run("git rev-parse --symbolic-full-names --abbrev-ref HEAD", shell=True, capture_output=True, text=True).stdout.strip() != 'master':
        pr_on_end = True

    example_paths = find_example_paths()
    results = []
    gather_default_environments(DEFAULT_CONFIG)
    create_environment_directories()
    run_count = os.environ.get('GITHUB_RUN_NUMBER', '0')
    run_count_str = str(run_count).zfill(5)

    os.makedirs(os.path.join(BUILD_DIR, run_count_str), exist_ok=True)

    # Iterate through examples
    count = 0
    for example_path in example_paths:
        config_file = get_configuration_file(example_path)
        print(f"Example ({example_path}) config [{config_file}]")
        environments = get_environments(config_file)
        print(f"Example environments: {environments}")
        for env in environments:
            build_dir = ""
            tweaked_example_path = example_path
            if (config_file == DEFAULT_CONFIG or env in DEFAULT_ENVIRONMENTS):
                build_dir = copy_example_to_env(example_path,env)
            else:
                build_dir = copy_example_to_extra_build(example_path)
                # tweaked_example_path = f"{example_path}-({env})"

            build_result = build_example(example_path, build_dir, env)
            out_log = format_log(build_result.stdout)
            err_log = format_log(build_result.stderr)

            log_file_path = os.path.join(BUILD_DIR, run_count_str, f"{os.path.basename(example_path)}_{env}.log")
            
            # Save the log file
            with open(log_file_path, 'w') as log_file:
                log_file.write(str(out_log) +"\n"+ str(err_log))
            log_file_path.replace("\\", "/")
            log_uri = convert_os_path_to_url(log_file_path)
            result_entry = create_result_entry(tweaked_example_path, env, build_result, f"https://github.com/HamzaHajeir/h4plugins/blob/{CI_BRANCH_NAME}/{log_uri}")
            results.append(result_entry)




    # Handle results
    report_table = create_report_table(results)
    report_path = save_report(report_table, run_count_str)

    switch_to_ci_branch()
    push_report(run_count_str)
    
    if pr_on_end:
        create_pull_request(report_path, run_count_str)

if __name__ == "__main__":
    main()

