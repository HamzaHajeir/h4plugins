#!/usr/bin/env python3
"""
PlatformIO INI Configuration Merger

Merges PlatformIO configuration files with three-phase overlay support:
1. REMOVE phase: Remove keys or specific lines from keys
2. SUBSTITUTE phase: Replace entire values
3. INSERT phase: Append lines to existing keys or create new keys

Usage:
    python platformio_merger.py --base base.ini --overlay overlay.ini --output merged.ini
    python platformio_merger.py --base base.ini --overlay overlay.ini --output merged.ini --verbose
    python platformio_merger.py --example [example_dir]
"""

import configparser
import re
import os
import sys
import argparse
from typing import Dict, List, Set, Tuple, Optional, Any

__version__ = "1.0.0"


class PlatformIOMerger:
    """PlatformIO INI configuration merger with three-phase overlay support."""
    
    # Regular expression for detecting partition markers
    MARKER_PATTERN = re.compile(r'^;\s*===+\s*(REMOVE|SUBSTITUTE|INSERT)\s*(?:===+\s*(.*))?$', re.IGNORECASE)
    
    # Regular expression for variable references ${section.key}
    VAR_REF_PATTERN = re.compile(r'\$\{([^}]+)\}')
    
    def __init__(self, verbose: bool = False):
        """
        Initialize the merger.
        
        Args:
            verbose: If True, print warnings and info to stdout
        """
        self.verbose = verbose
        self.warnings: List[str] = []
        self.errors: List[str] = []
        
    def log_warning(self, message: str):
        """Log a warning message."""
        self.warnings.append(message)
        if self.verbose:
            print(f"WARNING: {message}", file=sys.stderr)
    
    def log_error(self, message: str):
        """Log an error message."""
        self.errors.append(message)
        if self.verbose:
            print(f"ERROR: {message}", file=sys.stderr)
    
    def _create_parser(self) -> configparser.ConfigParser:
        """
        Create a configparser instance configured for PlatformIO files.
        
        Returns:
            ConfigParser instance with appropriate settings
        """
        return configparser.ConfigParser(
            allow_no_value=True,
            delimiters=('=',),  # PlatformIO uses '=' only
            comment_prefixes=(';', '#'),
            empty_lines_in_values=True,  # Important for multi-line values
            interpolation=None,  # We handle variable references ourselves
            strict=False  # Allow duplicate sections (will be merged)
        )

    def _parse_with_case_preservation(self, filename: str) -> configparser.ConfigParser:
        """
        Parse INI file while preserving case sensitivity and cleaning indentation.
        
        Args:
            filename: Path to INI file
            
        Returns:
            ConfigParser instance
        """
        parser = self._create_parser()
        parser.optionxform = str  # Preserve case of option names
        
        try:
            with open(filename, 'r') as f:
                content = f.read()
            
            # Parse the content
            parser.read_string(content)
            
            # Clean up indentation in all values
            for section in parser.sections():
                for key in parser[section]:
                    value = parser[section][key]
                    parser[section][key] = self._clean_value_indentation_on_read(value)
                    
        except configparser.Error as e:
            self.log_error(f"Failed to parse {filename}: {e}")
            raise
        except IOError as e:
            self.log_error(f"Failed to read {filename}: {e}")
            raise
        
        return parser
    
    def _clean_value_indentation_on_read(self, value: str) -> str:
        """
        Clean up indentation when reading a value.
        Removes all tabs - they'll be added back during formatting.
        
        Args:
            value: Value as read by configparser
            
        Returns:
            Cleaned value without tabs
        """
        if not value:
            return ""
        
        lines = value.split('\n')
        
        # Remove empty first line if present
        if lines and lines[0] == '':
            lines.pop(0)
        
        if not lines:
            return ""
        
        # Remove tabs from all lines
        cleaned_lines = [line.lstrip('\t') for line in lines]
        
        return '\n'.join(cleaned_lines)
    
    def _detect_partitions(self, content: str) -> Tuple[Dict[str, str], List[str]]:
        """
        Detect partitions in overlay file content.
        
        Args:
            content: Overlay file content as string
            
        Returns:
            Tuple of (partition_content_dict, order_list)
            partition_content_dict: {'REMOVE': content1, 'SUBSTITUTE': content2, 'INSERT': content3}
            order_list: List of partition names in order they appear
        """
        lines = content.splitlines()
        partitions: Dict[str, List[str]] = {'REMOVE': [], 'SUBSTITUTE': [], 'INSERT': []}
        current_partition = 'INSERT'  # Default if no markers
        order = []
        
        i = 0
        while i < len(lines):
            line = lines[i]
            match = self.MARKER_PATTERN.match(line)
            
            if match:
                partition_name = match.group(1).upper()
                if partition_name not in partitions:
                    self.log_warning(f"Unknown partition '{partition_name}', ignoring")
                    i += 1
                    continue
                
                # Start new partition
                current_partition = partition_name
                if partition_name not in order:
                    order.append(partition_name)
                
                # Skip the marker line
                i += 1
                continue
            
            # Add line to current partition
            if current_partition not in partitions:
                partitions[current_partition] = []
            partitions[current_partition].append(line)
            i += 1
        
        # Convert lists back to strings
        partition_content = {name: '\n'.join(lines) for name, lines in partitions.items()}
        
        # If no markers were found, the entire file is INSERT
        if not order:
            order = ['INSERT']
        
        return partition_content, order
    
    def _parse_overlay_file(self, filename: str) -> Tuple[Dict[str, configparser.ConfigParser], List[str]]:
        """
        Parse overlay file and split into partitions.
        
        Args:
            filename: Path to overlay file
            
        Returns:
            Tuple of (partition_parsers_dict, partition_order_list)
        """
        try:
            with open(filename, 'r') as f:
                content = f.read()
        except IOError as e:
            self.log_error(f"Failed to read overlay file {filename}: {e}")
            raise
        
        partition_content, order = self._detect_partitions(content)
        partition_parsers = {}
        
        for partition_name, content_str in partition_content.items():
            if not content_str.strip():
                # Empty partition, skip
                continue
            
            parser = self._create_parser()
            parser.optionxform = str
            
            try:
                parser.read_string(content_str)
                partition_parsers[partition_name] = parser
            except configparser.Error as e:
                self.log_error(f"Failed to parse {partition_name} partition: {e}")
                # Don't raise - continue with other partitions
                
        return partition_parsers, order
    
    def _strip_comments_and_whitespace(self, value: str) -> str:
        """
        Strip comments and whitespace from a value for matching purposes.
        
        Args:
            value: Original value string
            
        Returns:
            Cleaned value string
        """
        if not value:
            return ""
        
        lines = []
        for line in value.split('\n'):
            # Remove inline comments
            if ';' in line:
                line = line.split(';', 1)[0]
            if '#' in line:
                line = line.split('#', 1)[0]
            
            # Strip whitespace
            line = line.strip()
            if line:
                lines.append(line)
        
        return '\n'.join(lines)
    
    def _remove_lines_from_value(self, base_value: str, lines_to_remove: List[str]) -> str:
        """
        Remove specific lines from a multi-line value using substring matching.
        
        Args:
            base_value: Original multi-line value
            lines_to_remove: List of lines/patterns to remove
            
        Returns:
            Value with matching lines removed
        """
        if not base_value:
            return ""
        
        base_lines = []
        for line in base_value.split('\n'):
            cleaned_line = self._strip_comments_and_whitespace(line)
            
            if not cleaned_line:
                # Preserve empty lines in original
                base_lines.append(line)
                continue
            
            # Check if any removal pattern matches this line
            should_remove = False
            for pattern in lines_to_remove:
                pattern_clean = self._strip_comments_and_whitespace(pattern)
                if pattern_clean and pattern_clean in cleaned_line:
                    should_remove = True
                    break
            
            if not should_remove:
                base_lines.append(line)
        
        return '\n'.join(base_lines)
    
    def _append_lines_to_value(self, base_value: str, lines_to_add: List[str]) -> str:
        """
        Append lines to a value, ensuring no duplicates.
        
        Args:
            base_value: Original value (without tabs in internal representation)
            lines_to_add: Lines to append (without tabs)
            
        Returns:
            New value with appended lines (without tabs)
        """
        if not base_value:
            # If base is empty, return the lines to add
            return '\n'.join(lines_to_add) if lines_to_add else ""
        
        # Parse existing lines
        existing_lines = []
        for line in base_value.split('\n'):
            cleaned = self._strip_comments_and_whitespace(line)
            if cleaned:
                existing_lines.append(cleaned)
        
        # Add new lines that aren't already present
        result_lines = []
        for line in base_value.split('\n'):
            if line.strip():  # Keep non-empty lines
                result_lines.append(line)
        
        for line in lines_to_add:
            cleaned = self._strip_comments_and_whitespace(line)
            if cleaned and cleaned not in existing_lines:
                result_lines.append(line)
        
        return '\n'.join(result_lines)
    
    def _process_remove_phase(self, base_config: configparser.ConfigParser,
                            remove_config: configparser.ConfigParser) -> configparser.ConfigParser:
        """
        Process REMOVE phase.
        
        Args:
            base_config: Base configuration
            remove_config: Remove partition configuration
            
        Returns:
            Modified configuration
        """
        result_config = self._create_parser()
        result_config.optionxform = str
        
        # Copy all sections from base
        for section in base_config.sections():
            result_config.add_section(section)
            for key, value in base_config[section].items():
                result_config[section][key] = value
        
        # Process removals
        for section in remove_config.sections():
            if not result_config.has_section(section):
                continue
            
            for key, remove_value in remove_config[section].items():
                if not result_config.has_option(section, key):
                    continue
                
                base_value = result_config[section][key]
                
                if not remove_value or not remove_value.strip():
                    # Empty value in remove -> remove entire key
                    result_config.remove_option(section, key)
                    continue
                
                # Parse lines to remove
                lines_to_remove = []
                for line in remove_value.split('\n'):
                    cleaned = self._strip_comments_and_whitespace(line)
                    if cleaned:
                        lines_to_remove.append(cleaned)
                
                if not lines_to_remove:
                    continue
                
                # Remove matching lines
                new_value = self._remove_lines_from_value(base_value, lines_to_remove)
                
                if new_value.strip():
                    result_config[section][key] = new_value
                else:
                    # Value became empty, remove key
                    result_config.remove_option(section, key)
        
        return result_config
    
    def _process_substitute_phase(self, base_config: configparser.ConfigParser,
                                substitute_config: configparser.ConfigParser) -> configparser.ConfigParser:
        """
        Process SUBSTITUTE phase.
        
        Args:
            base_config: Current configuration (after remove phase)
            substitute_config: Substitute partition configuration
            
        Returns:
            Modified configuration
        """
        result_config = self._create_parser()
        result_config.optionxform = str
        
        # Copy all sections from base
        for section in base_config.sections():
            result_config.add_section(section)
            for key, value in base_config[section].items():
                result_config[section][key] = value
        
        # Process substitutions
        for section in substitute_config.sections():
            if not result_config.has_section(section):
                result_config.add_section(section)
            
            for key, new_value in substitute_config[section].items():
                # Clean the new value
                cleaned_value = self._clean_value_indentation_on_read(new_value)
                
                if result_config.has_option(section, key):
                    # Key exists, substitute it
                    result_config[section][key] = cleaned_value
                else:
                    # Key doesn't exist, create it with warning
                    result_config[section][key] = cleaned_value
                    self.log_warning(
                        f"SUBSTITUTE: Key '[{section}] {key}' didn't exist, created as new key"
                    )
        
        return result_config
    
    def _process_insert_phase(self, base_config: configparser.ConfigParser,
                            insert_config: configparser.ConfigParser) -> configparser.ConfigParser:
        """
        Process INSERT phase.
        
        Args:
            base_config: Current configuration (after substitute phase)
            insert_config: Insert partition configuration
            
        Returns:
            Modified configuration
        """
        result_config = self._create_parser()
        result_config.optionxform = str
        
        # Copy all sections from base
        for section in base_config.sections():
            result_config.add_section(section)
            for key, value in base_config[section].items():
                result_config[section][key] = value
        
        # Process insertions
        for section in insert_config.sections():
            if not result_config.has_section(section):
                result_config.add_section(section)
            
            for key, insert_value in insert_config[section].items():
                # Clean the insert value
                cleaned_insert = self._clean_value_indentation_on_read(insert_value)
                
                # Parse lines to insert
                lines_to_insert = []
                for line in cleaned_insert.split('\n'):
                    # Skip empty lines
                    if line.strip():
                        lines_to_insert.append(line)
                
                if not lines_to_insert:
                    continue
                
                if result_config.has_option(section, key):
                    # Key exists, append lines
                    base_value = result_config[section][key]
                    new_value = self._append_lines_to_value(base_value, lines_to_insert)
                    result_config[section][key] = new_value
                else:
                    # Key doesn't exist, create it
                    result_config[section][key] = cleaned_insert
        
        return result_config
    
    def _cleanup_empty_items(self, config: configparser.ConfigParser) -> configparser.ConfigParser:
        """
        Remove empty items and clean up variable references.
        
        Args:
            config: Configuration to clean up
            
        Returns:
            Cleaned configuration
        """
        # Track changes to know when we're done
        changed = True
        iterations = 0
        
        while changed and iterations < 100:  # Safety limit
            changed = False
            iterations += 1
            
            # First pass: remove empty keys
            sections_to_remove = []
            for section in config.sections():
                keys_to_remove = []
                for key, value in config[section].items():
                    if not value or not value.strip():
                        keys_to_remove.append(key)
                        changed = True
                
                for key in keys_to_remove:
                    config.remove_option(section, key)
                
                # Mark section for removal if empty
                if not list(config[section].keys()):
                    sections_to_remove.append(section)
            
            for section in sections_to_remove:
                config.remove_section(section)
                changed = True
            
            # Second pass: remove variable references to non-existent keys
            for section in config.sections():
                for key, value in config[section].items():
                    if not value:
                        continue
                    
                    # Find all variable references
                    new_value_parts = []
                    for line in value.split('\n'):
                        new_line = line
                        matches = list(self.VAR_REF_PATTERN.finditer(line))
                        
                        for match in reversed(matches):  # Process right to left
                            var_ref = match.group(0)  # ${section.key}
                            var_path = match.group(1)  # section.key
                            
                            # Check if referenced key exists and has value
                            ref_exists = False
                            if '.' in var_path:
                                ref_section, ref_key = var_path.split('.', 1)
                                if (config.has_section(ref_section) and 
                                    config.has_option(ref_section, ref_key) and
                                    config[ref_section][ref_key] and
                                    config[ref_section][ref_key].strip()):
                                    ref_exists = True
                            
                            if not ref_exists:
                                # Remove the reference
                                start, end = match.span()
                                new_line = new_line[:start] + new_line[end:]
                                new_line = new_line.strip()
                                changed = True
                        
                        if new_line.strip():
                            new_value_parts.append(new_line)
                    
                    # Update value if changed
                    new_value = '\n'.join(new_value_parts)
                    if new_value != value:
                        config[section][key] = new_value
                        changed = True
        
        if iterations >= 100:
            self.log_warning("Cleanup terminated after 100 iterations - possible circular reference")
        
        return config
    
    def _format_config(self, config: configparser.ConfigParser) -> str:
        """
        Format configuration with PlatformIO-style formatting.
        
        Args:
            config: Configuration to format
            
        Returns:
            Formatted string
        """
        lines = []
        
        for section in config.sections():
            lines.append(f"[{section}]")
            
            for key, value in config[section].items():
                if not value:
                    # Empty value
                    lines.append(f"{key} =")
                    continue
                
                value_lines = value.split('\n')
                if len(value_lines) == 1:
                    # Single-line value
                    lines.append(f"{key} = {value_lines[0]}")
                else:
                    # Multi-line value
                    lines.append(f"{key} = {value_lines[0]}")
                    for line in value_lines[1:]:
                        lines.append(f"\t{line}")
            
            lines.append("")  # Blank line between sections
        
        # Remove trailing blank lines
        while lines and lines[-1] == "":
            lines.pop()
        
        return '\n'.join(lines)
    
    def merge(self, base_file: str, overlay_file: str, output_file: str) -> Dict[str, Any]:
        """
        Merge base file with overlay file.
        
        Args:
            base_file: Path to base configuration file
            overlay_file: Path to overlay file
            output_file: Path to output file
            
        Returns:
            Dictionary with merge statistics and status
            
        Raises:
            IOError: If files cannot be read/written
            configparser.Error: If files have syntax errors
        """
        # Clear previous warnings/errors
        self.warnings = []
        self.errors = []
        
        try:
            # Parse base file
            base_config = self._parse_with_case_preservation(base_file)
            
            # Parse overlay file into partitions
            overlay_partitions, partition_order = self._parse_overlay_file(overlay_file)
            
            # Apply phases in order
            current_config = base_config
            
            if 'REMOVE' in partition_order and 'REMOVE' in overlay_partitions:
                if self.verbose:
                    print("Processing REMOVE phase...", file=sys.stderr)
                current_config = self._process_remove_phase(current_config, overlay_partitions['REMOVE'])
            
            if 'SUBSTITUTE' in partition_order and 'SUBSTITUTE' in overlay_partitions:
                if self.verbose:
                    print("Processing SUBSTITUTE phase...", file=sys.stderr)
                current_config = self._process_substitute_phase(current_config, overlay_partitions['SUBSTITUTE'])
            
            if 'INSERT' in partition_order and 'INSERT' in overlay_partitions:
                if self.verbose:
                    print("Processing INSERT phase...", file=sys.stderr)
                current_config = self._process_insert_phase(current_config, overlay_partitions['INSERT'])
            
            # Cleanup
            if self.verbose:
                print("Cleaning up empty items and references...", file=sys.stderr)
            current_config = self._cleanup_empty_items(current_config)
            
            # Write output
            formatted = self._format_config(current_config)
            with open(output_file, 'w') as f:
                f.write(formatted)
            
            # Calculate statistics
            stats = self._calculate_statistics(base_config, current_config)
            stats['warnings'] = len(self.warnings)
            stats['errors'] = len(self.errors)
            stats['partition_order'] = partition_order
            
            if self.verbose:
                print(f"Merged configuration written to {output_file}", file=sys.stderr)
                if self.warnings:
                    print(f"Generated {len(self.warnings)} warnings", file=sys.stderr)
            
            return {
                'success': True,
                'stats': stats,
                'warnings': self.warnings,
                'errors': self.errors
            }
            
        except Exception as e:
            self.log_error(f"Merge failed: {e}")
            return {
                'success': False,
                'stats': {},
                'warnings': self.warnings,
                'errors': self.errors
            }
    
    def _calculate_statistics(self, original: configparser.ConfigParser,
                            merged: configparser.ConfigParser) -> Dict[str, Any]:
        """
        Calculate merge statistics.
        
        Args:
            original: Original configuration
            merged: Merged configuration
            
        Returns:
            Dictionary with statistics
        """
        stats = {
            'original_sections': len(original.sections()),
            'merged_sections': len(merged.sections()),
            'original_keys': 0,
            'merged_keys': 0,
            'sections_added': 0,
            'sections_removed': 0,
            'keys_added': 0,
            'keys_removed': 0
        }
        
        # Count keys
        for section in original.sections():
            stats['original_keys'] += len(original[section])
        
        for section in merged.sections():
            stats['merged_keys'] += len(merged[section])
        
        # Calculate changes
        original_sections = set(original.sections())
        merged_sections = set(merged.sections())
        
        stats['sections_added'] = len(merged_sections - original_sections)
        stats['sections_removed'] = len(original_sections - merged_sections)
        
        # Count key changes by section
        for section in original_sections.intersection(merged_sections):
            original_keys = set(original[section].keys())
            merged_keys = set(merged[section].keys())
            
            stats['keys_added'] += len(merged_keys - original_keys)
            stats['keys_removed'] += len(original_keys - merged_keys)
        
        return stats


def merge_files(base_file: str, overlay_file: str, output_file: str, 
                verbose: bool = False, no_clobber: bool = False) -> bool:
    """
    Merge PlatformIO configuration files.
    
    Args:
        base_file: Path to base configuration file
        overlay_file: Path to overlay file
        output_file: Path to output file
        verbose: If True, print progress information
        no_clobber: If True, don't overwrite existing output file
        
    Returns:
        True if merge succeeded, False otherwise
    """
    # Check if output file exists and no_clobber is set
    if no_clobber and os.path.exists(output_file):
        print(f"ERROR: Output file '{output_file}' already exists and --no-clobber specified", 
              file=sys.stderr)
        return False
    
    # Create output directory if it doesn't exist
    output_dir = os.path.dirname(output_file)
    if output_dir and not os.path.exists(output_dir):
        try:
            os.makedirs(output_dir, exist_ok=True)
            if verbose:
                print(f"Created output directory: {output_dir}", file=sys.stderr)
        except OSError as e:
            print(f"ERROR: Failed to create output directory '{output_dir}': {e}", 
                  file=sys.stderr)
            return False
    
    merger = PlatformIOMerger(verbose=verbose)
    result = merger.merge(base_file, overlay_file, output_file)
    
    if verbose:
        if result['success']:
            stats = result['stats']
            print(f"Merge completed successfully!", file=sys.stderr)
            print(f"  Sections: {stats['original_sections']} → {stats['merged_sections']}", file=sys.stderr)
            print(f"  Keys: {stats['original_keys']} → {stats['merged_keys']}", file=sys.stderr)
            print(f"  Added: {stats['sections_added']} sections, {stats['keys_added']} keys", file=sys.stderr)
            print(f"  Removed: {stats['sections_removed']} sections, {stats['keys_removed']} keys", file=sys.stderr)
            if result['warnings']:
                print(f"  Generated {len(result['warnings'])} warnings", file=sys.stderr)
        else:
            print("Merge failed!", file=sys.stderr)
            if result['errors']:
                print(f"  Errors: {len(result['errors'])}", file=sys.stderr)
    
    return result['success']


def create_example_files(example_dir: str) -> bool:
    """
    Create example files for demonstration.
    
    Args:
        example_dir: Directory to create example files in
        
    Returns:
        True if successful, False otherwise
    """
    try:
        # Create directory if it doesn't exist
        os.makedirs(example_dir, exist_ok=True)
        
        # Example base configuration
        base_content = """[platformio]
default_envs = esp32

[common]
framework = arduino
monitor_speed = 921600
lib_deps = 
    https://github.com/owner/oldlib#1.0.0
    https://github.com/owner/commonlib#2.0.0
build_flags = 
    -DDEBUG
    -DVERBOSE

[env:esp32]
extends = common
board = esp32dev
board_build.partitions = default.csv
lib_deps = 
    ${common.lib_deps}
    https://github.com/owner/esplib#1.0.0
"""
        
        # Example overlay with all three phases
        overlay_content = """; === REMOVE ===
; Remove old library and debug flag
[common]
lib_deps = oldlib
build_flags = -DDEBUG

[env:esp32]
board_build.partitions = 

; === SUBSTITUTE ===
; Change monitor speed and board
[common]
monitor_speed = 115200

[env:esp32]
board = esp32devkitv1

; === INSERT ===
; Add new libraries and features
[common]
lib_deps = 
    https://github.com/owner/newlib#3.0.0
build_flags = 
    -DOPTIMIZED

[env:esp32]
lib_deps = 
    https://github.com/owner/esplib2#1.0.0
build_flags = 
    -DBOARD_HAS_PSRAM

[env:pico]
board = pico_w
platform = raspberrypi
"""
        
        # Write files
        base_file = os.path.join(example_dir, "base.ini")
        overlay_file = os.path.join(example_dir, "overlay.ini")
        
        with open(base_file, 'w') as f:
            f.write(base_content)
        
        with open(overlay_file, 'w') as f:
            f.write(overlay_content)
        
        # Try to merge them as a demonstration
        output_file = os.path.join(example_dir, "merged.ini")
        success = merge_files(base_file, overlay_file, output_file, verbose=False)
        
        if success:
            print(f"Created example files in: {example_dir}")
            print(f"  - {base_file}")
            print(f"  - {overlay_file}")
            print(f"  - {output_file}")
            print()
            print("To test the merger, run:")
            print(f"  python platformio_merger.py --base {base_file} --overlay {overlay_file} --output {example_dir}/test.ini --verbose")
        else:
            print(f"Warning: Created example files but merge test failed", file=sys.stderr)
            print(f"Example files created in: {example_dir}")
        
        return True
        
    except Exception as e:
        print(f"ERROR: Failed to create example files: {e}", file=sys.stderr)
        return False


def validate_file(filename: str, check_readable: bool = True) -> bool:
    """
    Validate that a file exists and is accessible.
    
    Args:
        filename: Path to file
        check_readable: If True, check read permission
        
    Returns:
        True if file is valid, False otherwise
    """
    if not os.path.exists(filename):
        print(f"ERROR: File '{filename}' does not exist", file=sys.stderr)
        return False
    
    if not os.path.isfile(filename):
        print(f"ERROR: '{filename}' is not a file", file=sys.stderr)
        return False
    
    if check_readable and not os.access(filename, os.R_OK):
        print(f"ERROR: File '{filename}' is not readable", file=sys.stderr)
        return False
    
    return True


def main():
    """Main entry point for command-line interface."""
    parser = argparse.ArgumentParser(
        description='Merge PlatformIO INI configuration files with three-phase overlay support',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s --base config.ini --overlay changes.ini --output merged.ini
  %(prog)s --base config.ini --overlay changes.ini --output merged.ini --verbose
  %(prog)s --base config.ini --overlay changes.ini --output merged.ini --no-clobber
  %(prog)s --example
  %(prog)s --example ./my_example_dir
  
Overlay file format:
  Overlay files can contain three sections marked with comments:
  
  ; === REMOVE ===
  [section]
  key_to_remove = value_pattern
  
  ; === SUBSTITUTE ===
  [section]
  key = new_value
  
  ; === INSERT ===
  [section]
  key = value_to_append
  
  If no markers are found, the entire file is treated as INSERT phase.
"""
    )
    
    # Main arguments
    parser.add_argument('--base', '-b', 
                       help='Base PlatformIO configuration file',
                       metavar='FILE')
    parser.add_argument('--overlay', '-l',
                       help='Overlay file with REMOVE/SUBSTITUTE/INSERT partitions',
                       metavar='FILE')
    parser.add_argument('--output', '-o',
                       help='Output file path',
                       metavar='FILE')
    
    # Options
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Enable verbose output')
    parser.add_argument('--no-clobber', '-n', action='store_true',
                       help='Do not overwrite existing output file')
    
    # Info commands
    parser.add_argument('--example', nargs='?', const=True,
                       help='Generate example files. If PATH is provided, create in PATH, '
                            'otherwise create in script_directory/piomerger_example/')
    parser.add_argument('--version', action='version',
                       version=f'%(prog)s {__version__}')
    
    args = parser.parse_args()
    
    # Handle --example command
    if args.example:
        if args.example is True:
            # Use default example directory (script-relative)
            script_dir = os.path.dirname(os.path.abspath(__file__))
            example_dir = os.path.join(script_dir, "piomerger_example")
        else:
            # Use user-specified directory
            example_dir = os.path.abspath(args.example)
        
        success = create_example_files(example_dir)
        sys.exit(0 if success else 1)
    
    # Validate required arguments for merge operation
    if not args.base or not args.overlay or not args.output:
        parser.error("--base, --overlay, and --output are required for merge operation")
    
    # Expand ~ in file paths
    args.base = os.path.expanduser(args.base)
    args.overlay = os.path.expanduser(args.overlay)
    args.output = os.path.expanduser(args.output)
    
    # Validate input files
    if not validate_file(args.base):
        sys.exit(1)
    if not validate_file(args.overlay):
        sys.exit(1)
    
    # Perform merge
    try:
        success = merge_files(
            base_file=args.base,
            overlay_file=args.overlay,
            output_file=args.output,
            verbose=args.verbose,
            no_clobber=args.no_clobber
        )
        sys.exit(0 if success else 1)
    except KeyboardInterrupt:
        print("\nOperation cancelled by user", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"ERROR: Unexpected error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()