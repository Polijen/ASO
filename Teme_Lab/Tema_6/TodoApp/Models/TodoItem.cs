using System.ComponentModel.DataAnnotations;
 
namespace TodoApp.Models;

public enum Priority
{
    Low,
    Medium,
    High
}
 
public class TodoItem
{
    public int Id { get; set; }
 
    [Required]
    [StringLength(200, MinimumLength = 1)]
    public string Title { get; set; } = string.Empty;
 
    public bool IsCompleted { get; set; }
 
    public DateTime CreatedAt { get; set; } = DateTime.UtcNow;

	public DateTime? DueDate { get; set; }	

    public Priority TaskPriority { get; set; } = Priority.Medium;
}
